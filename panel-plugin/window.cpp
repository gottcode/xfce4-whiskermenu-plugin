/*
 * Copyright (C) 2013-2020 Graeme Gott <graeme@gottcode.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window.h"

#include "applications-page.h"
#include "category-button.h"
#include "command.h"
#include "favorites-page.h"
#include "launcher-view.h"
#include "plugin.h"
#include "profile-picture.h"
#include "recent-page.h"
#include "resizer-widget.h"
#include "search-page.h"
#include "settings.h"
#include "slot.h"

#include <exo/exo.h>
#include <gdk/gdkkeysyms.h>
#include <libxfce4ui/libxfce4ui.h>

#include <ctime>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static void grab_pointer(GtkWidget* widget)
{
	if (wm_settings->stay_on_focus_out)
	{
		return;
	}

	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* seat = gdk_display_get_default_seat(display);
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_seat_grab(seat, window, GDK_SEAT_CAPABILITY_ALL_POINTING, true, nullptr, nullptr, nullptr, nullptr);
}

static void ungrab_pointer()
{
	if (wm_settings->stay_on_focus_out)
	{
		return;
	}

	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* seat = gdk_display_get_default_seat(display);
	gdk_seat_ungrab(seat);
}

//-----------------------------------------------------------------------------

WhiskerMenu::Window::Window(Plugin* plugin) :
	m_plugin(plugin),
	m_window(nullptr),
	m_search_cover(GTK_STACK_TRANSITION_TYPE_OVER_DOWN),
	m_search_uncover(GTK_STACK_TRANSITION_TYPE_UNDER_UP),
	m_sidebar_size_group(nullptr),
	m_geometry{0,0,wm_settings->menu_width,wm_settings->menu_height},
	m_layout_left(true),
	m_layout_bottom(true),
	m_layout_categories_alternate(false),
	m_layout_search_alternate(false),
	m_layout_commands_alternate(false),
	m_supports_alpha(false)
{
	// Create the window
	m_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_widget_set_name(GTK_WIDGET(m_window), "whiskermenu-window");
	// Untranslated window title to allow window managers to identify it; not visible to users.
	gtk_window_set_title(m_window, "Whisker Menu");
	gtk_window_set_modal(m_window, true);
	gtk_window_set_decorated(m_window, false);
	gtk_window_set_skip_taskbar_hint(m_window, true);
	gtk_window_set_skip_pager_hint(m_window, true);
	gtk_window_set_type_hint(m_window, GDK_WINDOW_TYPE_HINT_POPUP_MENU);
	gtk_window_stick(m_window);
	gtk_widget_add_events(GTK_WIDGET(m_window), GDK_BUTTON_PRESS_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_STRUCTURE_MASK);
	g_signal_connect_slot(m_window, "enter-notify-event", &Window::on_enter_notify_event, this);
	g_signal_connect_slot(m_window, "leave-notify-event", &Window::on_leave_notify_event, this);
	g_signal_connect_slot(m_window, "button-press-event", &Window::on_button_press_event, this);
	g_signal_connect_slot(m_window, "button-release-event", &Window::on_button_release_event, this);
	g_signal_connect_slot(m_window, "key-press-event", &Window::on_key_press_event, this);
	g_signal_connect_slot(m_window, "key-press-event", &Window::on_key_press_event_after, this, true);
	g_signal_connect_slot(m_window, "map-event", &Window::on_map_event, this);
	g_signal_connect_slot(m_window, "state-flags-changed", &Window::on_state_flags_changed_event, this);
	g_signal_connect_slot(m_window, "configure-event", &Window::on_configure_event, this);
	g_signal_connect(m_window, "delete_event", G_CALLBACK(&gtk_widget_hide_on_delete), nullptr);

	// Create the border of the window
	GtkWidget* frame = gtk_frame_new(nullptr);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_add(GTK_CONTAINER(m_window), frame);

	// Create window contents stack
	m_window_stack = GTK_STACK(gtk_stack_new());
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(m_window_stack));

	// Create loading message
	m_window_load_spinner = GTK_SPINNER(gtk_spinner_new());
	gtk_widget_set_halign(GTK_WIDGET(m_window_load_spinner), GTK_ALIGN_CENTER);
	gtk_widget_set_valign(GTK_WIDGET(m_window_load_spinner), GTK_ALIGN_CENTER);
	gtk_stack_add_named(m_window_stack, GTK_WIDGET(m_window_load_spinner), "load");

	// Create the profile picture
	m_profilepic = new ProfilePicture(this);

	// Create the username label
	const gchar* name = g_get_real_name();
	if (g_strcmp0(name, "Unknown") == 0)
	{
		name = g_get_user_name();
	}
	gchar* username = g_markup_printf_escaped("<b><big>%s</big></b>", name);
	m_username = GTK_LABEL(gtk_label_new(nullptr));
	gtk_label_set_markup(m_username, username);
	gtk_widget_set_halign(GTK_WIDGET(m_username), GTK_ALIGN_START);
	g_free(username);

	// Create action buttons
	for (int i = 0; i < 9; ++i)
	{
		m_commands_button[i] = wm_settings->command[i]->get_button();
		m_command_slots[i] = g_signal_connect_slot<GtkButton*>(m_commands_button[i], "clicked", &Window::hide, this);
	}

	m_resizer = new ResizerWidget(m_window);

	// Create search entry
	m_search_entry = GTK_ENTRY(gtk_search_entry_new());
	g_signal_connect_slot<GtkSearchEntry*>(m_search_entry, "search-changed", &Window::search, this);

	// Create favorites
	m_favorites = new FavoritesPage(this);

	GIcon* icon = g_themed_icon_new("user-bookmarks");
	m_favorites_button = new CategoryButton(icon, _("Favorites"));
	g_object_unref(icon);
	g_signal_connect_slot<GtkToggleButton*>(m_favorites_button->get_widget(), "toggled", &Window::favorites_toggled, this);

	// Create recent
	m_recent = new RecentPage(this);

	icon = g_themed_icon_new("document-open-recent");
	m_recent_button = new CategoryButton(icon, _("Recently Used"));
	g_object_unref(icon);
	m_recent_button->join_group(m_favorites_button);
	g_signal_connect_slot<GtkToggleButton*>(m_recent_button->get_widget(), "toggled", &Window::recent_toggled, this);

	// Create applications
	m_applications = new ApplicationsPage(this);

	// Create search results
	m_search_results = new SearchPage(this);

	// Handle default page
	if (!wm_settings->display_recent)
	{
		m_default_button = m_favorites_button;
		m_default_page = m_favorites;
	}
	else
	{
		m_default_button = m_recent_button;
		m_default_page = m_recent;
	}

	// Create box for packing children
	m_vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	gtk_container_set_border_width(GTK_CONTAINER(m_vbox), 2);
	gtk_stack_add_named(m_window_stack, GTK_WIDGET(m_vbox), "contents");

	// Create box for packing commands
	m_commands_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	m_commands_spacer = gtk_label_new(nullptr);
	gtk_box_pack_start(m_commands_box, m_commands_spacer, true, true, 0);
	for (auto command : m_commands_button)
	{
		gtk_box_pack_start(m_commands_box, command, false, false, 0);
	}

	// Create box for packing username, commands, and resize widget
	m_title_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	gtk_box_pack_start(m_vbox, GTK_WIDGET(m_title_box), false, false, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_profilepic->get_widget()), false, false, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_username), true, true, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_commands_box), false, false, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_resizer->get_widget()), false, false, 0);

	// Add search to layout
	m_search_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
	gtk_box_pack_start(m_vbox, GTK_WIDGET(m_search_box), false, true, 0);
	gtk_box_pack_start(m_search_box, GTK_WIDGET(m_search_entry), true, true, 0);

	// Create box for packing launcher pages and sidebar
	m_contents_stack = GTK_STACK(gtk_stack_new());
	m_contents_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
	gtk_stack_add_named(m_contents_stack, GTK_WIDGET(m_contents_box), "contents");
	gtk_stack_add_named(m_contents_stack, m_search_results->get_widget(), "search");
	gtk_box_pack_start(m_vbox, GTK_WIDGET(m_contents_stack), true, true, 0);

	// Create box for packing launcher pages
	m_panels_stack = GTK_STACK(gtk_stack_new());
	gtk_box_pack_start(m_contents_box, GTK_WIDGET(m_panels_stack), true, true, 0);
	gtk_stack_add_named(m_panels_stack, m_favorites->get_widget(), "favorites");
	gtk_stack_add_named(m_panels_stack, m_recent->get_widget(), "recent");
	gtk_stack_add_named(m_panels_stack, m_applications->get_widget(), "applications");

	// Create box for packing sidebar
	m_sidebar_buttons = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_box_pack_start(m_sidebar_buttons, m_favorites_button->get_widget(), false, false, 0);
	gtk_box_pack_start(m_sidebar_buttons, m_recent_button->get_widget(), false, false, 0);
	gtk_box_pack_start(m_sidebar_buttons, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), false, false, 4);

	m_sidebar = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(nullptr, nullptr));
	gtk_box_pack_start(m_contents_box, GTK_WIDGET(m_sidebar), false, false, 0);
	gtk_scrolled_window_set_shadow_type(m_sidebar, GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy(m_sidebar, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(m_sidebar), GTK_WIDGET(m_sidebar_buttons));

	// Show widgets
	gtk_widget_show_all(frame);
	m_default_button->set_active(true);

	// Resize to last known size
	gtk_window_set_default_size(m_window, m_geometry.width, m_geometry.height);

	// Handle transparency
	gtk_widget_set_app_paintable(GTK_WIDGET(m_window), true);
	g_signal_connect_slot(m_window, "draw", &Window::on_draw_event, this);
	g_signal_connect_slot(m_window, "screen-changed", &Window::on_screen_changed_event, this);
	on_screen_changed_event(GTK_WIDGET(m_window), nullptr);

	// Load applications
	m_applications->load();

	g_object_ref_sink(m_window);
}

//-----------------------------------------------------------------------------

WhiskerMenu::Window::~Window()
{
	for (int i = 0; i < 9; ++i)
	{
		g_signal_handler_disconnect(m_commands_button[i], m_command_slots[i]);
		gtk_container_remove(GTK_CONTAINER(m_commands_box), m_commands_button[i]);
	}

	delete m_applications;
	delete m_search_results;
	delete m_recent;
	delete m_favorites;

	delete m_profilepic;
	delete m_resizer;

	delete m_favorites_button;
	delete m_recent_button;

	gtk_widget_destroy(GTK_WIDGET(m_window));
	g_object_unref(m_window);
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::hide()
{
	ungrab_pointer();

	// Scroll categories to top
	GtkAdjustment* adjustment = gtk_scrolled_window_get_vadjustment(m_sidebar);
	gtk_adjustment_set_value(adjustment, gtk_adjustment_get_lower(adjustment));

	// Reset any pressed category buttons
	unset_pressed_category();

	// Hide command buttons to remove active border
	for (auto command : m_commands_button)
	{
		gtk_widget_set_visible(command, false);
	}

	// Hide window
	gtk_widget_hide(GTK_WIDGET(m_window));

	// Switch back to default page
	show_default_page();
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::show(const Position position)
{
	// Handle switching view types
	m_search_results->update_view();
	m_favorites->update_view();
	m_recent->update_view();
	m_applications->update_view();

	// Handle showing tooltips
	if (wm_settings->launcher_show_tooltip)
	{
		m_search_results->get_view()->show_tooltips();
		m_favorites->get_view()->show_tooltips();
		m_recent->get_view()->show_tooltips();
		m_applications->get_view()->show_tooltips();
	}
	else
	{
		m_search_results->get_view()->hide_tooltips();
		m_favorites->get_view()->hide_tooltips();
		m_recent->get_view()->hide_tooltips();
		m_applications->get_view()->hide_tooltips();
	}
	m_profilepic->reset_tooltip();

	// Make sure commands are valid and visible
	for (auto command : wm_settings->command)
	{
		command->check();
	}

	// Make sure recent item count is within max
	m_recent->enforce_item_count();

	// Make sure recent button is only visible when tracked
	gtk_widget_set_visible(m_recent_button->get_widget(), wm_settings->recent_items_max);

	// Make sure applications list is current; does nothing unless list has changed
	if (m_applications->load())
	{
		set_loaded();
	}
	else
	{
		m_plugin->set_loaded(false);
		gtk_stack_set_visible_child_name(m_window_stack, "load");
		gtk_spinner_start(m_window_load_spinner);
	}

	// Update default page
	if (wm_settings->display_recent)
	{
		m_default_button = m_recent_button;
		m_default_page = m_recent;
	}
	else
	{
		m_default_button = m_favorites_button;
		m_default_page = m_favorites;
	}
	show_default_page();

	// Make sure icon sizes are correct
	m_favorites_button->reload_icon_size();
	m_recent_button->reload_icon_size();
	m_applications->reload_category_icon_size();

	m_search_results->get_view()->reload_icon_size();
	m_favorites->get_view()->reload_icon_size();
	m_recent->get_view()->reload_icon_size();
	m_applications->get_view()->reload_icon_size();

	GdkScreen* screen = nullptr;
	int parent_x = 0, parent_y = 0, parent_w = 0, parent_h = 0;
	if (position != PositionAtCursor)
	{
		// Wait up to half a second for auto-hidden panels to be shown
		clock_t end = clock() + (CLOCKS_PER_SEC / 2);
		GtkWidget* parent = m_plugin->get_button();
		GtkWindow* parent_window = GTK_WINDOW(gtk_widget_get_toplevel(parent));
		gtk_window_get_position(parent_window, &parent_x, &parent_y);
		while ((parent_x == -9999) && (parent_y == -9999) && (clock() < end))
		{
			while (gtk_events_pending())
			{
				gtk_main_iteration();
			}
			gtk_window_get_position(parent_window, &parent_x, &parent_y);
		}

		// Fetch parent geometry
		if (!gtk_widget_get_realized(parent))
		{
			gtk_widget_realize(parent);
		}
		GdkWindow* window = gtk_widget_get_window(parent);
		gdk_window_get_origin(window, &parent_x, &parent_y);
		screen = gdk_window_get_screen(window);
		parent_w = gdk_window_get_width(window);
		parent_h = gdk_window_get_height(window);
	}
	else
	{
		GdkDisplay* display = gdk_display_get_default();
		GdkSeat* seat = gdk_display_get_default_seat(display);
		GdkDevice* device = gdk_seat_get_pointer(seat);
		gdk_device_get_position(device, &screen, &parent_x, &parent_y);
	}

	// Fetch screen geomtry
	GdkRectangle monitor;
	GdkMonitor* monitor_gdk = gdk_display_get_monitor_at_point(gdk_display_get_default(), parent_x, parent_y);
	gdk_monitor_get_geometry(monitor_gdk, &monitor);

	// Prevent window from being larger than screen
	if (m_geometry.width > monitor.width)
	{
		m_geometry.width = monitor.width;
		gtk_window_resize(m_window, m_geometry.width, m_geometry.height);
	}
	if (m_geometry.height > monitor.height)
	{
		m_geometry.height = monitor.height;
		gtk_window_resize(m_window, m_geometry.width, m_geometry.height);
	}

	// Find window position
	bool layout_left = ((2 * (parent_x - monitor.x)) + parent_w) < monitor.width;
	bool layout_bottom = ((2 * (parent_y - monitor.y)) + (parent_h / 2)) > monitor.height;
	if (position != PositionVertical)
	{
		m_geometry.x = layout_left ? parent_x : (parent_x + parent_w - m_geometry.width);
		m_geometry.y = layout_bottom ? (parent_y - m_geometry.height) : (parent_y + parent_h);
	}
	else
	{
		m_geometry.x = layout_left ? (parent_x + parent_w) : (parent_x - m_geometry.width);
		m_geometry.y = layout_bottom ? (parent_y + parent_h - m_geometry.height) : parent_y;
	}

	// Prevent window from leaving screen
	m_geometry.x = CLAMP(m_geometry.x, monitor.x, monitor.x + monitor.width - m_geometry.width);
	m_geometry.y = CLAMP(m_geometry.y, monitor.y, monitor.y + monitor.height - m_geometry.height);

	// Move window
	gtk_window_move(m_window, m_geometry.x, m_geometry.y);

	// Set corner for resizer
	if (layout_left)
	{
		if (layout_bottom)
		{
			m_resizer->set_corner(ResizerWidget::TopRight);
		}
		else
		{
			m_resizer->set_corner(ResizerWidget::BottomRight);
		}
	}
	else
	{
		if (layout_bottom)
		{
			m_resizer->set_corner(ResizerWidget::TopLeft);
		}
		else
		{
			m_resizer->set_corner(ResizerWidget::BottomLeft);
		}
	}

	// Relayout window if necessary
	if (gtk_widget_get_direction(GTK_WIDGET(m_window)) == GTK_TEXT_DIR_RTL)
	{
		layout_left = !layout_left;
	}

	if ((m_layout_left != layout_left)
			|| (m_layout_bottom != layout_bottom)
			|| (m_layout_categories_alternate != wm_settings->position_categories_alternate)
			|| (m_layout_search_alternate != wm_settings->position_search_alternate)
			|| (m_layout_commands_alternate != wm_settings->position_commands_alternate))
	{
		m_layout_left = layout_left;
		m_layout_bottom = layout_bottom;
		m_layout_categories_alternate = wm_settings->position_categories_alternate;
		m_layout_search_alternate = wm_settings->position_search_alternate;
		m_layout_commands_alternate = wm_settings->position_commands_alternate;
		update_layout();
	}

	if (!m_sidebar_size_group && m_layout_commands_alternate && wm_settings->category_show_name)
	{
		m_sidebar_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
		gtk_size_group_add_widget(m_sidebar_size_group, GTK_WIDGET(m_sidebar));
		gtk_size_group_add_widget(m_sidebar_size_group, GTK_WIDGET(m_commands_box));
	}
	else if (m_sidebar_size_group && (!m_layout_commands_alternate || !wm_settings->category_show_name))
	{
		gtk_size_group_remove_widget(m_sidebar_size_group, GTK_WIDGET(m_sidebar));
		gtk_size_group_remove_widget(m_sidebar_size_group, GTK_WIDGET(m_commands_box));
		g_object_unref(m_sidebar_size_group);
		m_sidebar_size_group = nullptr;
	}

	// Show window
	gtk_widget_show(GTK_WIDGET(m_window));
	gtk_window_move(m_window, m_geometry.x, m_geometry.y);
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::save()
{
	if (wm_settings->menu_width != m_geometry.width)
	{
		wm_settings->menu_width = m_geometry.width;
		wm_settings->set_modified();
	}
	if (wm_settings->menu_height != m_geometry.height)
	{
		wm_settings->menu_height = m_geometry.height;
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::on_context_menu_destroyed()
{
	grab_pointer(GTK_WIDGET(m_window));
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::set_categories(const std::vector<CategoryButton*>& categories)
{
	CategoryButton* last_button = m_recent_button;
	for (auto button : categories)
	{
		button->join_group(last_button);
		last_button = button;
		gtk_box_pack_start(m_sidebar_buttons, button->get_widget(), false, false, 0);
		g_signal_connect_slot<GtkToggleButton*>(button->get_widget(), "toggled", &Window::category_toggled, this);
	}

	// Position "All Applications" above divider
	if (!categories.empty())
	{
		gtk_box_reorder_child(m_sidebar_buttons, categories.front()->get_widget(), 2);
	}

	show_default_page();
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::set_items()
{
	m_search_results->set_menu_items(m_applications->get_view()->get_model());
	m_favorites->set_menu_items();
	m_recent->set_menu_items();

	// Handle switching to favorites are added
	GtkTreeModel* favorites_model = m_favorites->get_view()->get_model();
	g_signal_connect_slot<GtkTreeModel*, GtkTreePath*, GtkTreeIter*>(favorites_model, "row-inserted", &Window::show_favorites, this);
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::set_loaded()
{
	// Hide loading spinner
	gtk_spinner_stop(m_window_load_spinner);
	gtk_stack_set_visible_child_name(m_window_stack, "contents");

	// Focus search entry
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));

	// Show panel button
	m_plugin->set_loaded(true);

	// Check in case of plugin reload
	check_scrollbar_needed();
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::unset_items()
{
	m_search_results->unset_menu_items();
	m_favorites->unset_menu_items();
	m_recent->unset_menu_items();
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_enter_notify_event(GtkWidget*, GdkEvent* event)
{
	GdkEventCrossing* crossing_event = reinterpret_cast<GdkEventCrossing*>(event);
	if ((crossing_event->detail == GDK_NOTIFY_INFERIOR)
			|| (crossing_event->mode == GDK_CROSSING_GRAB)
			|| (crossing_event->mode == GDK_CROSSING_GTK_GRAB))
	{
		return false;
	}

	grab_pointer(GTK_WIDGET(m_window));

	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_leave_notify_event(GtkWidget*, GdkEvent* event)
{
	GdkEventCrossing* crossing_event = reinterpret_cast<GdkEventCrossing*>(event);
	if ((crossing_event->detail == GDK_NOTIFY_INFERIOR)
			|| (crossing_event->mode != GDK_CROSSING_NORMAL))
	{
		return false;
	}

	grab_pointer(GTK_WIDGET(m_window));

	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_button_press_event(GtkWidget*, GdkEvent* event)
{
	if (wm_settings->stay_on_focus_out)
	{
		return false;
	}

	// Hide menu if user clicks outside
	GdkEventButton* button_event = reinterpret_cast<GdkEventButton*>(event);
	if ((button_event->x_root <= m_geometry.x)
			|| (button_event->x_root >= m_geometry.x + m_geometry.width)
			|| (button_event->y_root <= m_geometry.y)
			|| (button_event->y_root >= m_geometry.y + m_geometry.height))
	{
		hide();
	}
	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_button_release_event(GtkWidget*, GdkEvent*)
{
	unset_pressed_category();
	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_key_press_event(GtkWidget* widget, GdkEvent* event)
{
	GdkEventKey* key_event = reinterpret_cast<GdkEventKey*>(event);

	// Hide if escape is pressed and there is no text in search entry
	if ( (key_event->keyval == GDK_KEY_Escape) && exo_str_is_empty(gtk_entry_get_text(m_search_entry)) )
	{
		hide();
		return true;
	}

	Page* page = nullptr;
	if (gtk_stack_get_visible_child(m_contents_stack) == m_search_results->get_widget())
	{
		page = m_search_results;
	}
	else if (m_favorites_button->get_active())
	{
		page = m_favorites;
	}
	else if (m_recent_button->get_active())
	{
		page = m_recent;
	}
	else
	{
		page = m_applications;
	}
	GtkWidget* view = page->get_view()->get_widget();

	// Allow keyboard navigation out of treeview
	if ((key_event->keyval == GDK_KEY_Left) || (key_event->keyval == GDK_KEY_Right))
	{
		if (GTK_IS_TREE_VIEW(view) && ((widget == view) || (gtk_window_get_focus(m_window) == view)))
		{
			gtk_widget_grab_focus(m_favorites_button->get_widget());
			page->reset_selection();
		}
	}

	// Make up and down keys scroll current list of applications from search
	if ((key_event->keyval == GDK_KEY_Up) || (key_event->keyval == GDK_KEY_Down))
	{
		GtkWidget* search = GTK_WIDGET(m_search_entry);
		if ((widget == search) || (gtk_window_get_focus(m_window) == search))
		{
			GtkTreePath* path = page->get_view()->get_cursor();
			if (path)
			{
				page->get_view()->select_path(path);
			}
			gtk_widget_grab_focus(view);
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_key_press_event_after(GtkWidget* widget, GdkEvent* event)
{
	// Pass unhandled key presses to search entry
	GtkWidget* search_entry = GTK_WIDGET(m_search_entry);
	if ((widget != search_entry) && (gtk_window_get_focus(m_window) != search_entry))
	{
		GdkEventKey* key_event = reinterpret_cast<GdkEventKey*>(event);
		if (key_event->is_modifier)
		{
			return false;
		}
		gtk_widget_grab_focus(search_entry);
		gtk_window_propagate_key_event(m_window, key_event);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_map_event(GtkWidget*, GdkEvent*)
{
	m_favorites->reset_selection();

	gtk_window_set_keep_above(m_window, true);

	// Track mouse clicks outside of menu
	grab_pointer(GTK_WIDGET(m_window));

	// Focus search entry
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));

	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_state_flags_changed_event(GtkWidget* widget, GtkStateFlags)
{
	// Refocus and raise window if visible
	if (gtk_widget_get_visible(widget))
	{
		gtk_window_present(m_window);
	}

	return false;
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_configure_event(GtkWidget*, GdkEvent* event)
{
	GdkEventConfigure* configure_event = reinterpret_cast<GdkEventConfigure*>(event);
	if (configure_event->width && configure_event->height)
	{
		m_geometry.x = configure_event->x;
		m_geometry.y = configure_event->y;
		m_geometry.width = configure_event->width;
		m_geometry.height = configure_event->height;
	}

	check_scrollbar_needed();

	return false;
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::on_screen_changed_event(GtkWidget* widget, GdkScreen*)
{
	GdkScreen* screen = gtk_widget_get_screen(widget);
	GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
	if (!visual || (wm_settings->menu_opacity == 100))
	{
		visual = gdk_screen_get_system_visual(screen);
		m_supports_alpha = false;
	}
	else
	{
		m_supports_alpha = true;
	}
	gtk_widget_set_visual(widget, visual);
}

//-----------------------------------------------------------------------------

gboolean WhiskerMenu::Window::on_draw_event(GtkWidget* widget, cairo_t* cr)
{
	if (!gtk_widget_get_realized(widget))
	{
		gtk_widget_realize(widget);
	}

	GtkStyleContext* context = gtk_widget_get_style_context(widget);
	const double width = gtk_widget_get_allocated_width(widget);
	const double height = gtk_widget_get_allocated_height(widget);

	if (m_supports_alpha)
	{
		cairo_surface_t* background = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
		cairo_t* cr_background = cairo_create(background);
		cairo_set_operator(cr_background, CAIRO_OPERATOR_SOURCE);
		gtk_render_background(context, cr_background, 0.0, 0.0, width, height);
		cairo_destroy(cr_background);

		cairo_set_source_surface(cr, background, 0.0, 0.0);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint_with_alpha(cr, wm_settings->menu_opacity / 100.0);

		cairo_surface_destroy(background);
	}
	else
	{
		gtk_render_background(context, cr, 0.0, 0.0, width, height);
	}

	return false;
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::check_scrollbar_needed()
{
	// Find height of sidebar buttons
	int height = 0;
	gtk_widget_get_preferred_height(GTK_WIDGET(m_sidebar_buttons), nullptr, &height);

	// Always show scrollbar if sidebar is shorter than buttons
	int allocated = gtk_widget_get_allocated_height(GTK_WIDGET(m_sidebar));
	if ((allocated > height) || (allocated == 1))
	{
		gtk_scrolled_window_set_policy(m_sidebar, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	}
	else
	{
		gtk_scrolled_window_set_policy(m_sidebar, GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	}
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::favorites_toggled()
{
	m_favorites->reset_selection();
	gtk_stack_set_visible_child_name(m_panels_stack, "favorites");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::recent_toggled()
{
	m_recent->reset_selection();
	gtk_stack_set_visible_child_name(m_panels_stack, "recent");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::category_toggled()
{
	m_applications->reset_selection();
	gtk_stack_set_visible_child_name(m_panels_stack, "applications");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::show_favorites()
{
	// Switch to favorites panel
	m_favorites_button->set_active(true);

	// Clear search entry
	gtk_entry_set_text(m_search_entry, "");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::show_default_page()
{
	// Switch to favorites panel
	m_default_button->set_active(true);

	// Clear search entry
	gtk_entry_set_text(m_search_entry, "");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::search()
{
	// Fetch search string
	const gchar* text = gtk_entry_get_text(m_search_entry);
	if (exo_str_is_empty(text))
	{
		text = nullptr;
	}

	if (text)
	{
		// Show search results
		gtk_stack_set_visible_child_full(m_contents_stack, "search", m_search_cover);
	}
	else
	{
		// Show active panel
		gtk_stack_set_visible_child_full(m_contents_stack, "contents", m_search_uncover);
	}

	// Apply filter
	m_search_results->set_filter(text);
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::unset_pressed_category()
{
	// Force a state change on sidebar buttons
	gtk_widget_set_sensitive(GTK_WIDGET(m_sidebar_buttons), false);
	gtk_widget_set_sensitive(GTK_WIDGET(m_sidebar_buttons), true);
}

//-----------------------------------------------------------------------------

void WhiskerMenu::Window::update_layout()
{
	// Set vertical position of commands
	g_object_ref(m_commands_box);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(m_commands_box))), GTK_WIDGET(m_commands_box));
	if (m_layout_commands_alternate)
	{
		gtk_box_pack_start(m_search_box, GTK_WIDGET(m_commands_box), false, false, 0);

		if (m_layout_left == m_layout_categories_alternate)
		{
			gtk_box_reorder_child(m_search_box, GTK_WIDGET(m_commands_box), 0);
			gtk_box_reorder_child(m_search_box, GTK_WIDGET(m_search_entry), 1);
		}
	}
	else
	{
		gtk_box_pack_start(m_title_box, GTK_WIDGET(m_commands_box), false, false, 0);
	}
	g_object_unref(m_commands_box);

	// Arrange horizontal order of profile picture, username, resizer, and commands
	if (m_layout_left && m_layout_commands_alternate)
	{
		gtk_widget_set_halign(GTK_WIDGET(m_username), GTK_ALIGN_START);

		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_profilepic->get_widget()), 0);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_username), 1);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_resizer->get_widget()), 2);

		for (int i = 0; i < 9; ++i)
		{
			gtk_box_reorder_child(m_commands_box, m_commands_button[i], i);
		}
	}
	else if (m_layout_commands_alternate)
	{
		gtk_widget_set_halign(GTK_WIDGET(m_username), GTK_ALIGN_END);

		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_profilepic->get_widget()), 2);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_username), 1);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_resizer->get_widget()), 0);

		for (int i = 0; i < 9; ++i)
		{
			gtk_box_reorder_child(m_commands_box, m_commands_button[i], 8 - i);
		}
	}
	else if (m_layout_left)
	{
		gtk_widget_set_halign(GTK_WIDGET(m_username), GTK_ALIGN_START);

		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_profilepic->get_widget()), 0);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_username), 1);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_commands_box), 2);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_resizer->get_widget()), 3);

		for (int i = 0; i < 9; ++i)
		{
			gtk_box_reorder_child(m_commands_box, m_commands_button[i], i);
		}
	}
	else
	{
		gtk_widget_set_halign(GTK_WIDGET(m_username), GTK_ALIGN_END);

		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_profilepic->get_widget()), 3);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_username), 2);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_commands_box), 1);
		gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_resizer->get_widget()), 0);

		for (int i = 0; i < 9; ++i)
		{
			gtk_box_reorder_child(m_commands_box, m_commands_button[i], 8 - i);
		}
	}

	// Arrange horizontal order of applications and sidebar
	if (m_layout_left != m_layout_categories_alternate)
	{
		gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_panels_stack), 0);
		gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_sidebar), 1);

		gtk_box_reorder_child(m_commands_box, m_commands_spacer, 0);
	}
	else
	{
		gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_panels_stack), 1);
		gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_sidebar), 0);

		gtk_box_reorder_child(m_commands_box, m_commands_spacer, 9);
	}

	// Arrange vertical order of header, applications, and search
	if (m_layout_bottom && m_layout_search_alternate)
	{
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_title_box), 0);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_contents_stack), 1);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_search_box), 2);

		m_search_cover = GTK_STACK_TRANSITION_TYPE_OVER_UP;
		m_search_uncover = GTK_STACK_TRANSITION_TYPE_UNDER_DOWN;
	}
	else if (m_layout_search_alternate)
	{
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_title_box), 2);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_contents_stack), 1);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_search_box), 0);

		m_search_cover = GTK_STACK_TRANSITION_TYPE_OVER_DOWN;
		m_search_uncover = GTK_STACK_TRANSITION_TYPE_UNDER_UP;
	}
	else if (m_layout_bottom)
	{
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_title_box), 0);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_search_box), 1);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_contents_stack), 2);

		m_search_cover = GTK_STACK_TRANSITION_TYPE_OVER_DOWN;
		m_search_uncover = GTK_STACK_TRANSITION_TYPE_UNDER_UP;
	}
	else
	{
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_title_box), 2);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_search_box), 1);
		gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_contents_stack), 0);

		m_search_cover = GTK_STACK_TRANSITION_TYPE_OVER_UP;
		m_search_uncover = GTK_STACK_TRANSITION_TYPE_UNDER_DOWN;
	}
}

//-----------------------------------------------------------------------------
