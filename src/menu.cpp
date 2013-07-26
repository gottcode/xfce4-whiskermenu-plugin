// Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this library.  If not, see <http://www.gnu.org/licenses/>.


#include "menu.hpp"

#include "applications_page.hpp"
#include "favorites_page.hpp"
#include "launcher_view.hpp"
#include "recent_page.hpp"
#include "resizer_widget.hpp"
#include "search_page.hpp"
#include "section_button.hpp"

#include <ctime>

extern "C"
{
#include <exo/exo.h>
#include <gdk/gdkkeysyms.h>
#include <libxfce4ui/libxfce4ui.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static GtkButton* new_action_button(const gchar* icon, const gchar* text)
{
	GtkButton* button = GTK_BUTTON(gtk_button_new());
	gtk_button_set_relief(button, GTK_RELIEF_NONE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(button), text);

	GtkWidget* image = gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));

	return button;
}

//-----------------------------------------------------------------------------

Menu::Menu(XfceRc* settings) :
	m_window(NULL),
	m_layout_left(true),
	m_layout_bottom(true),
	m_modified(false)
{
	m_geometry.x = 0;
	m_geometry.y = 0;
	m_geometry.width = 400;
	m_geometry.height = 500;

	// Create the window
	m_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_widget_set_name(GTK_WIDGET(m_window), "whiskermenu-window");
	gtk_window_set_modal(m_window, true);
	gtk_window_set_decorated(m_window, false);
	gtk_window_set_skip_taskbar_hint(m_window, true);
	gtk_window_set_skip_pager_hint(m_window, true);
	gtk_window_stick(m_window);
	gtk_widget_add_events(GTK_WIDGET(m_window), GDK_BUTTON_PRESS_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_STRUCTURE_MASK);
	g_signal_connect(m_window, "enter-notify-event", G_CALLBACK(Menu::on_enter_notify_event_slot), this);
	g_signal_connect(m_window, "leave-notify-event", G_CALLBACK(Menu::on_leave_notify_event_slot), this);
	g_signal_connect(m_window, "focus-in-event", G_CALLBACK(on_focus_in_event_slot), this);
	g_signal_connect(m_window, "button-press-event", G_CALLBACK(Menu::on_button_press_event_slot), this);
	g_signal_connect(m_window, "key-press-event", G_CALLBACK(Menu::on_key_press_event_slot), this);
	g_signal_connect_after(m_window, "key-press-event", G_CALLBACK(Menu::on_key_press_event_after_slot), this);
	g_signal_connect(m_window, "map-event", G_CALLBACK(Menu::on_map_event_slot), this);
	g_signal_connect(m_window, "configure-event", G_CALLBACK(Menu::on_configure_event_slot), this);

	// Create the border of the window
	GtkWidget* frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_add(GTK_CONTAINER(m_window), frame);

	// Create the username label
	const gchar* name = g_get_real_name();
	if (g_strcmp0(name, "Unknown") == 0)
	{
		name = g_get_user_name();
	}
	gchar* username = g_markup_printf_escaped("<b><big>%s</big></b>", name);
	m_username = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_markup(m_username, username);
	gtk_misc_set_alignment(GTK_MISC(m_username), 0.0f, 0.5f);
	g_free(username);

	// Create action buttons
	m_settings_button = new_action_button("preferences-desktop", _("All Settings"));
	g_signal_connect(m_settings_button, "clicked", G_CALLBACK(Menu::launch_settings_manager_slot), this);

	m_lock_screen_button = new_action_button("system-lock-screen", _("Lock Screen"));
	g_signal_connect(m_lock_screen_button, "clicked", G_CALLBACK(Menu::lock_screen_slot), this);

	m_log_out_button = new_action_button("system-log-out", _("Log Out"));
	g_signal_connect(m_log_out_button, "clicked", G_CALLBACK(Menu::log_out_slot), this);

	m_resizer = new ResizerWidget(m_window);

	// Create search entry
	m_search_entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_icon_from_stock(m_search_entry, GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);
	gtk_entry_set_icon_activatable(m_search_entry, GTK_ENTRY_ICON_SECONDARY, false);
	g_signal_connect(m_search_entry, "changed", G_CALLBACK(Menu::search_slot), this);

	// Create favorites
	m_favorites = new FavoritesPage(settings, this);

	m_favorites_button = new SectionButton("user-bookmarks", _("Favorites"));
	g_signal_connect(m_favorites_button->get_button(), "toggled", G_CALLBACK(Menu::favorites_toggled_slot), this);

	// Create recent
	m_recent = new RecentPage(settings, this);

	m_recent_button = new SectionButton("document-open-recent", _("Recently Used"));
	m_recent_button->set_group(m_favorites_button->get_group());
	g_signal_connect(m_recent_button->get_button(), "toggled", G_CALLBACK(Menu::recent_toggled_slot), this);

	// Create applications
	m_applications = new ApplicationsPage(this);

	// Create search results
	m_search_results = new SearchPage(this);

	// Create box for packing children
	m_vbox = GTK_BOX(gtk_vbox_new(false, 6));
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(m_vbox));
	gtk_container_set_border_width(GTK_CONTAINER(m_vbox), 2);

	// Create box for packing username and action buttons
	m_title_box = GTK_BOX(gtk_hbox_new(false, 0));
	gtk_box_pack_start(m_vbox, GTK_WIDGET(m_title_box), false, false, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_username), true, true, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_settings_button), false, false, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_lock_screen_button), false, false, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_log_out_button), false, false, 0);
	gtk_box_pack_start(m_title_box, GTK_WIDGET(m_resizer->get_widget()), false, false, 0);

	// Add search to layout
	gtk_box_pack_start(m_vbox, GTK_WIDGET(m_search_entry), false, true, 0);

	// Create box for packing launcher pages and sidebar
	m_contents_box = GTK_BOX(gtk_hbox_new(false, 6));
	gtk_box_pack_start(m_vbox, GTK_WIDGET(m_contents_box), true, true, 0);
	gtk_box_pack_start(m_contents_box, m_search_results->get_widget(), true, true, 0);

	// Create box for packing launcher pages
	m_panels_box = GTK_BOX(gtk_vbox_new(false, 0));
	gtk_box_pack_start(m_contents_box, GTK_WIDGET(m_panels_box), true, true, 0);
	gtk_box_pack_start(m_panels_box, m_favorites->get_widget(), true, true, 0);
	gtk_box_pack_start(m_panels_box, m_recent->get_widget(), true, true, 0);
	gtk_box_pack_start(m_panels_box, m_applications->get_widget(), true, true, 0);

	// Create box for packing sidebar
	m_sidebar_box = GTK_BOX(gtk_vbox_new(false, 0));
	gtk_box_pack_start(m_sidebar_box, GTK_WIDGET(m_favorites_button->get_button()), false, false, 0);
	gtk_box_pack_start(m_sidebar_box, GTK_WIDGET(m_recent_button->get_button()), false, false, 0);
	gtk_box_pack_start(m_sidebar_box, gtk_hseparator_new(), false, true, 0);

	m_sidebar = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
	gtk_box_pack_start(m_contents_box, GTK_WIDGET(m_sidebar), false, false, 0);
	gtk_scrolled_window_set_shadow_type(m_sidebar, GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy(m_sidebar, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	GtkWidget* viewport = gtk_viewport_new(gtk_scrolled_window_get_hadjustment(m_sidebar),
		gtk_scrolled_window_get_vadjustment(m_sidebar));
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(m_sidebar), viewport);
	gtk_container_add(GTK_CONTAINER(viewport), GTK_WIDGET(m_sidebar_box));

	// Populate app menu
	m_applications->load_applications();

	// Show widgets
	gtk_widget_show_all(GTK_WIDGET(m_vbox));
	gtk_widget_hide(m_recent->get_widget());
	gtk_widget_hide(m_applications->get_widget());
	gtk_widget_hide(m_search_results->get_widget());
	m_favorites_button->set_active(true);
	gtk_widget_show(frame);

	// Resize to last known size
	if (settings)
	{
		m_geometry.width = std::max(300, xfce_rc_read_int_entry(settings, "menu-width", m_geometry.width));
		m_geometry.height = std::max(400, xfce_rc_read_int_entry(settings, "menu-height", m_geometry.height));
	}
	gtk_window_set_default_size(m_window, m_geometry.width, m_geometry.height);

	g_object_ref_sink(m_window);
}

//-----------------------------------------------------------------------------

Menu::~Menu()
{
	delete m_applications;
	delete m_search_results;
	delete m_recent;
	delete m_favorites;

	delete m_resizer;
	g_object_unref(m_window);
}

//-----------------------------------------------------------------------------

void Menu::hide()
{
	gdk_pointer_ungrab(gtk_get_current_event_time());

	// Hide window
	gtk_widget_hide(GTK_WIDGET(m_window));

	// Reset mouse cursor by forcing favorites to hide
	gtk_widget_hide(m_favorites->get_widget());

	// Switch back to favorites
	show_favorites();
}

//-----------------------------------------------------------------------------

void Menu::show(GtkWidget* parent, bool horizontal)
{
	// Make sure icon sizes are correct
	m_favorites_button->reload_icon_size();
	m_recent_button->reload_icon_size();

	m_search_results->get_view()->reload_icon_size();
	m_favorites->get_view()->reload_icon_size();
	m_recent->get_view()->reload_icon_size();
	m_applications->get_view()->reload_icon_size();

	// Make sure applications list is current; does nothing unless list has changed
	m_applications->load_applications();

	// Reset mouse cursor by forcing favorites to hide
	gtk_widget_show(m_favorites->get_widget());

	GdkScreen* screen = NULL;
	int parent_x = 0, parent_y = 0, parent_w = 0, parent_h = 0;
	if (parent != NULL)
	{
		// Wait up to half a second for auto-hidden panels to be shown
		clock_t end = clock() + (CLOCKS_PER_SEC / 2);
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
		gdk_display_get_pointer(display, &screen, &parent_x, &parent_y, NULL);
	}

	// Fetch screen geomtry
	GdkRectangle monitor;
	int monitor_num = gdk_screen_get_monitor_at_point(screen, parent_x, parent_y);
	gdk_screen_get_monitor_geometry(screen, monitor_num, &monitor);

	// Find window position
	bool layout_left = ((2 * (parent_x - monitor.x)) + parent_w) < monitor.width;
	if (horizontal)
	{
		m_geometry.x = layout_left ? parent_x : (parent_x + parent_w - m_geometry.width);
	}
	else
	{
		m_geometry.x = layout_left ? (parent_x + parent_w) : (parent_x - m_geometry.width);
	}

	bool layout_bottom = ((2 * (parent_y - monitor.y)) + (parent_h / 2)) > monitor.height;
	if (horizontal)
	{
		m_geometry.y = layout_bottom ? (parent_y - m_geometry.height) : (parent_y + parent_h);
	}
	else
	{
		m_geometry.y = layout_bottom ? (parent_y + parent_h - m_geometry.height) : parent_y;
	}

	// Prevent window from leaving screen
	int monitor_r = monitor.x + monitor.width;
	if (m_geometry.x < monitor.x)
	{
		m_geometry.width += m_geometry.x - monitor.x;
		m_geometry.x = monitor.x;
		gtk_window_resize(GTK_WINDOW(m_window), m_geometry.width, m_geometry.height);
	}
	else if ((m_geometry.x + m_geometry.width) > monitor_r)
	{
		m_geometry.width = monitor_r - m_geometry.x;
		gtk_window_resize(GTK_WINDOW(m_window), m_geometry.width, m_geometry.height);
	}

	int monitor_b = monitor.y + monitor.height;
	if (m_geometry.y < monitor.y)
	{
		m_geometry.height += m_geometry.y - monitor.y;
		m_geometry.y = monitor.y;
		gtk_window_resize(GTK_WINDOW(m_window), m_geometry.width, m_geometry.height);
	}
	else if ((m_geometry.y + m_geometry.height) > monitor_b)
	{
		m_geometry.height = monitor_b - m_geometry.y;
		gtk_window_resize(GTK_WINDOW(m_window), m_geometry.width, m_geometry.height);
	}

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
	if (layout_left != m_layout_left)
	{
		m_layout_left = layout_left;
		if (m_layout_left)
		{
			gtk_misc_set_alignment(GTK_MISC(m_username), 0.0f, 0.5f);

			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_username), 0);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_settings_button), 1);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_lock_screen_button), 2);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_log_out_button), 3);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_resizer->get_widget()), 4);

			gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_panels_box), 1);
			gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_sidebar), 2);
		}
		else
		{
			gtk_misc_set_alignment(GTK_MISC(m_username), 1.0f, 0.5f);

			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_username), 4);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_settings_button), 3);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_lock_screen_button), 2);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_log_out_button), 1);
			gtk_box_reorder_child(m_title_box, GTK_WIDGET(m_resizer->get_widget()), 0);

			gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_panels_box), 2);
			gtk_box_reorder_child(m_contents_box, GTK_WIDGET(m_sidebar), 1);
		}
	}

	if (layout_bottom != m_layout_bottom)
	{
		m_layout_bottom = layout_bottom;
		if (m_layout_bottom)
		{
			gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_title_box), 0);
			gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_search_entry), 1);
			gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_contents_box), 2);
		}
		else
		{
			gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_title_box), 2);
			gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_search_entry), 1);
			gtk_box_reorder_child(m_vbox, GTK_WIDGET(m_contents_box), 0);
		}
	}

	// Show window
	gtk_widget_show(GTK_WIDGET(m_window));
	gtk_window_move(m_window, m_geometry.x, m_geometry.y);
}

//-----------------------------------------------------------------------------

void Menu::save(XfceRc* settings)
{
	if (settings)
	{
		m_favorites->save(settings);
		m_recent->save(settings);
		xfce_rc_write_int_entry(settings, "menu-width", m_geometry.width);
		xfce_rc_write_int_entry(settings, "menu-height", m_geometry.height);
		m_modified = false;
	}
}

//-----------------------------------------------------------------------------

void Menu::set_categories(const std::vector<SectionButton*>& categories)
{
	for (std::vector<SectionButton*>::const_iterator i = categories.begin(), end = categories.end(); i != end; ++i)
	{
		(*i)->set_group(m_recent_button->get_group());
		gtk_box_pack_start(m_sidebar_box, GTK_WIDGET((*i)->get_button()), false, false, 0);
		g_signal_connect((*i)->get_button(), "toggled", G_CALLBACK(Menu::category_toggled_slot), this);
	}
	gtk_widget_show_all(GTK_WIDGET(m_sidebar_box));

	show_favorites();
}

//-----------------------------------------------------------------------------

void Menu::set_items()
{
	m_search_results->set_menu_items(m_applications->get_model());
	m_favorites->set_menu_items();
	m_recent->set_menu_items();

	// Handle switching to favorites are added
	GtkTreeModel* favorites_model = m_favorites->get_view()->get_model();
	g_signal_connect(favorites_model, "row-inserted", G_CALLBACK(Menu::show_favorites_slot), this);
}

//-----------------------------------------------------------------------------

void Menu::set_modified()
{
	m_modified = true;
}

//-----------------------------------------------------------------------------

void Menu::unset_items()
{
	m_search_results->unset_menu_items();
	m_favorites->unset_menu_items();
	m_recent->unset_menu_items();
}

//-----------------------------------------------------------------------------

bool Menu::on_enter_notify_event(GdkEventCrossing* event)
{
	if ( (event->detail == GDK_NOTIFY_INFERIOR)
			|| (event->mode == GDK_CROSSING_GRAB)
			|| (event->mode == GDK_CROSSING_GTK_GRAB) )
	{
		return false;
	}

	// Don't grab cursor over menu
	if ((event->x_root >= m_geometry.x) && (event->x_root < (m_geometry.x + m_geometry.width))
			&& (event->y_root >= m_geometry.y) && (event->y_root < (m_geometry.y + m_geometry.height)))
	{
		if (gdk_pointer_is_grabbed())
		{
			gdk_pointer_ungrab(gtk_get_current_event_time());
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

bool Menu::on_leave_notify_event(GdkEventCrossing* event)
{
	if ( (event->detail == GDK_NOTIFY_INFERIOR)
			|| (event->mode != GDK_CROSSING_NORMAL) )
	{
		return false;
	}

	// Track mouse clicks outside of menu
	if ((event->x_root <= m_geometry.x) || (event->x_root >= m_geometry.x + m_geometry.width)
			|| (event->y_root <= m_geometry.y) || (event->y_root >= m_geometry.y + m_geometry.height))
	{
		gdk_pointer_grab(gtk_widget_get_window(GTK_WIDGET(m_window)), true,
				GdkEventMask(
					GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
				),
				NULL, NULL, event->time);
	}

	return false;
}

//-----------------------------------------------------------------------------

bool Menu::on_focus_in_event()
{
	gdk_pointer_grab(gtk_widget_get_window(GTK_WIDGET(m_window)), true,
			GdkEventMask(
				GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
				GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			),
			NULL, NULL, gtk_get_current_event_time());
	return false;
}

//-----------------------------------------------------------------------------

bool Menu::on_button_press_event(GdkEventButton* event)
{
	// Hide menu if user clicks outside
	if ((event->x_root <= m_geometry.x) || (event->x_root >= m_geometry.x + m_geometry.width)
			|| (event->y_root <= m_geometry.y) || (event->y_root >= m_geometry.y + m_geometry.height))
	{
		hide();
	}
	return false;
}

//-----------------------------------------------------------------------------

bool Menu::on_key_press_event(GtkWidget* widget, GdkEventKey* event)
{
	// Hide if escape is pressed and there is no text in search entry
	if ( (event->keyval == GDK_Escape) && exo_str_is_empty(gtk_entry_get_text(m_search_entry)) )
	{
		hide();
		return true;
	}

	// Make up and down keys always scroll current list of applications
	if ((event->keyval == GDK_KEY_Up) || (event->keyval == GDK_KEY_Down))
	{
		GtkWidget* view = NULL;
		if (gtk_widget_get_visible(m_search_results->get_widget()))
		{
			view = m_search_results->get_view()->get_widget();
		}
		else if (m_favorites_button->get_active())
		{
			view = m_favorites->get_view()->get_widget();
		}
		else if (m_recent_button->get_active())
		{
			view = m_recent->get_view()->get_widget();
		}
		else
		{
			view = m_applications->get_view()->get_widget();
		}

		if ((widget != view) && (gtk_window_get_focus(m_window) != view))
		{
			gtk_widget_grab_focus(view);
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

bool Menu::on_key_press_event_after(GtkWidget* widget, GdkEventKey* event)
{
	// Pass unhandled key presses to search entry
	GtkWidget* search_entry = GTK_WIDGET(m_search_entry);
	if ((widget != search_entry) && (gtk_window_get_focus(m_window) != search_entry))
	{
		gtk_widget_grab_focus(search_entry);
		gtk_window_propagate_key_event(m_window, event);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

bool Menu::on_map_event()
{
	m_favorites->reset_selection();

	gtk_window_set_keep_above(m_window, true);

	// Track mouse clicks outside of menu
	gdk_pointer_grab(gtk_widget_get_window(GTK_WIDGET(m_window)), true,
			GdkEventMask(
				GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
				GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			),
			NULL, NULL, gtk_get_current_event_time());

	// Focus search entry
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));

	return false;
}

//-----------------------------------------------------------------------------

bool Menu::on_configure_event(GdkEventConfigure* event)
{
	if (event->width && event->height)
	{
		m_geometry.x = event->x;
		m_geometry.y = event->y;
		m_geometry.width = event->width;
		m_geometry.height = event->height;
	}
	return false;
}

//-----------------------------------------------------------------------------

void Menu::favorites_toggled()
{
	m_favorites->reset_selection();
	gtk_widget_hide(m_recent->get_widget());
	gtk_widget_hide(m_applications->get_widget());
	gtk_widget_show_all(m_favorites->get_widget());
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void Menu::recent_toggled()
{
	m_recent->reset_selection();
	gtk_widget_hide(m_favorites->get_widget());
	gtk_widget_hide(m_applications->get_widget());
	gtk_widget_show_all(m_recent->get_widget());
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void Menu::category_toggled()
{
	m_applications->reset_selection();
	gtk_widget_hide(m_favorites->get_widget());
	gtk_widget_hide(m_recent->get_widget());
	gtk_widget_show_all(m_applications->get_widget());
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void Menu::show_favorites()
{
	// Switch to favorites panel
	m_favorites_button->set_active(true);

	// Clear search entry
	gtk_entry_set_text(m_search_entry, "");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void Menu::search()
{
	// Fetch search string
	gchar* filter_string = NULL;
	const gchar* text = gtk_entry_get_text(m_search_entry);
	if (!exo_str_is_empty(text))
	{
		gchar* normalized = g_utf8_normalize(text, -1, G_NORMALIZE_DEFAULT);
		filter_string = g_utf8_casefold(normalized, -1);
		g_free(normalized);
	}

	// Update search entry icon
	bool visible = filter_string != NULL;
	gtk_entry_set_icon_from_stock(m_search_entry, GTK_ENTRY_ICON_SECONDARY, !visible ? GTK_STOCK_FIND : GTK_STOCK_CLEAR);
	gtk_entry_set_icon_activatable(m_search_entry, GTK_ENTRY_ICON_SECONDARY, visible);

	if (visible)
	{
		// Show search results
		gtk_widget_hide(GTK_WIDGET(m_sidebar));
		gtk_widget_hide(GTK_WIDGET(m_panels_box));
		gtk_widget_show(m_search_results->get_widget());
	}
	else
	{
		// Show active panel
		gtk_widget_hide(m_search_results->get_widget());
		gtk_widget_show(GTK_WIDGET(m_panels_box));
		gtk_widget_show(GTK_WIDGET(m_sidebar));
	}

	// Apply filter
	m_search_results->set_filter(visible ? filter_string : NULL);
	g_free(filter_string);
}

//-----------------------------------------------------------------------------

void Menu::launch_settings_manager()
{
	hide();

	GError* error = NULL;
	if (g_spawn_command_line_async("xfce4-settings-manager", &error) == false)
	{
		xfce_dialog_show_error(NULL, error, _("Failed to open settings manager."));
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------

void Menu::lock_screen()
{
	hide();

	GError* error = NULL;
	if (g_spawn_command_line_async("xflock4", &error) == false)
	{
		xfce_dialog_show_error(NULL, error, _("Failed to lock screen."));
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------

void Menu::log_out()
{
	hide();

	GError* error = NULL;
	if (g_spawn_command_line_async("xfce4-session-logout", &error) == false)
	{
		xfce_dialog_show_error(NULL, error, _("Failed to log out."));
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------
