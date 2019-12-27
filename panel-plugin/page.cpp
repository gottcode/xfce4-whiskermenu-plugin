/*
 * Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019 Graeme Gott <graeme@gottcode.org>
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

#include "page.h"

#include "favorites-page.h"
#include "launcher.h"
#include "launcher-icon-view.h"
#include "launcher-tree-view.h"
#include "recent-page.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

#include <libxfce4ui/libxfce4ui.h>

#include <glib/gstdio.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

Page::Page(Window* window) :
	m_window(window),
	m_selected_launcher(NULL),
	m_drag_enabled(true),
	m_launcher_dragged(false),
	m_reorderable(false)
{
	// Create view
	create_view();

	// Add scrolling to view
	m_widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_widget), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_widget), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_widget), m_view->get_widget());
	g_object_ref_sink(m_widget);
}

//-----------------------------------------------------------------------------

Page::~Page()
{
	delete m_view;
	gtk_widget_destroy(m_widget);
	g_object_unref(m_widget);
}

//-----------------------------------------------------------------------------

void Page::reset_selection()
{
	m_view->collapse_all();

	// Set keyboard focus on first item
	GtkTreeModel* model = m_view->get_model();
	GtkTreeIter iter;
	if (model && gtk_tree_model_get_iter_first(model, &iter))
	{
		GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
		m_view->set_cursor(path);
		gtk_tree_path_free(path);
	}

	// Scroll to top
	GtkAdjustment* adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(m_view->get_widget()));
	gtk_adjustment_set_value(adjustment, gtk_adjustment_get_lower(adjustment));

	// Clear selection
	m_view->clear_selection();
}

//-----------------------------------------------------------------------------

void Page::update_view()
{
	if ((dynamic_cast<LauncherIconView*>(m_view) != 0) == wm_settings->view_as_icons)
	{
		return;
	}

	LauncherView* view = m_view;
	create_view();
	m_view->set_model(view->get_model());
	delete view;

	gtk_container_add(GTK_CONTAINER(m_widget), m_view->get_widget());
	gtk_widget_show_all(m_widget);

	view_created();
}

//-----------------------------------------------------------------------------

void Page::set_reorderable(bool reorderable)
{
	m_reorderable = reorderable;
	if (m_reorderable)
	{
		const GtkTargetEntry row_targets[] = {
			{ g_strdup("GTK_TREE_MODEL_ROW"), GTK_TARGET_SAME_WIDGET, 0 },
			{ g_strdup("text/uri-list"), GTK_TARGET_OTHER_APP, 1 }
		};

		m_view->set_drag_source(GDK_BUTTON1_MASK,
				row_targets, 2,
				GdkDragAction(GDK_ACTION_MOVE | GDK_ACTION_COPY));

		m_view->set_drag_dest(row_targets, 1,
				GDK_ACTION_MOVE);

		g_free(row_targets[0].target);
		g_free(row_targets[1].target);
	}
	else
	{
		const GtkTargetEntry row_targets[] = {
			{ g_strdup("text/uri-list"), GTK_TARGET_OTHER_APP, 1 }
		};

		m_view->set_drag_source(GDK_BUTTON1_MASK,
				row_targets, 1,
				GDK_ACTION_COPY);

		m_view->unset_drag_dest();

		g_free(row_targets[0].target);
	}
}

//-----------------------------------------------------------------------------

void Page::create_view()
{
	if (wm_settings->view_as_icons)
	{
		m_view = new LauncherIconView();
		g_signal_connect(m_view->get_widget(), "item-activated", G_CALLBACK(&Page::item_activated_slot), this);
	}
	else
	{
		m_view = new LauncherTreeView();
		g_signal_connect(m_view->get_widget(), "row-activated", G_CALLBACK(&Page::row_activated_slot), this);
		g_signal_connect_swapped(m_view->get_widget(), "start-interactive-search", G_CALLBACK(gtk_widget_grab_focus), m_window->get_search_entry());
	}
	g_signal_connect_slot(m_view->get_widget(), "button-press-event", &Page::view_button_press_event, this);
	g_signal_connect_slot(m_view->get_widget(), "button-release-event", &Page::view_button_release_event, this);
	g_signal_connect_slot(m_view->get_widget(), "drag-data-get", &Page::view_drag_data_get, this);
	g_signal_connect_slot(m_view->get_widget(), "drag-end", &Page::view_drag_end, this);
	g_signal_connect_slot(m_view->get_widget(), "popup-menu", &Page::view_popup_menu_event, this);
	set_reorderable(m_reorderable);
}

//-----------------------------------------------------------------------------

bool Page::remember_launcher(Launcher*)
{
	return true;
}

//-----------------------------------------------------------------------------

void Page::launcher_activated(GtkTreePath* path)
{
	GtkTreeIter iter;
	GtkTreeModel* model = m_view->get_model();
	gtk_tree_model_get_iter(model, &iter, path);

	// Find element
	Element* element = NULL;
	gtk_tree_model_get(model, &iter, LauncherView::COLUMN_LAUNCHER, &element, -1);
	if (!element)
	{
		return;
	}

	// Add to recent
	if (element->get_type() == Launcher::Type)
	{
		Launcher* launcher = static_cast<Launcher*>(element);
		if (remember_launcher(launcher))
		{
			m_window->get_recent()->add(launcher);
		}
	}

	// Hide window
	m_window->hide();

	// Execute app
	element->run(gtk_widget_get_screen(m_widget));
}

//-----------------------------------------------------------------------------

void Page::launcher_action_activated(GtkMenuItem* menuitem, DesktopAction* action)
{
	g_assert(m_selected_launcher != NULL);

	// Add to recent
	if (remember_launcher(m_selected_launcher))
	{
		m_window->get_recent()->add(m_selected_launcher);
	}

	// Hide window
	m_window->hide();

	// Execute app
	m_selected_launcher->run(gtk_widget_get_screen(GTK_WIDGET(menuitem)), action);
}

//-----------------------------------------------------------------------------

gboolean Page::view_button_press_event(GtkWidget*, GdkEvent* event)
{
	GdkEventButton* button_event = reinterpret_cast<GdkEventButton*>(event);

	m_launcher_dragged = false;

	GtkTreePath* path = m_view->get_path_at_pos(button_event->x, button_event->y);
	if (!path)
	{
		return false;
	}

	if (button_event->button == 3)
	{
		create_context_menu(path, event);
		return true;
	}
	else if (button_event->button != 1)
	{
		gtk_tree_path_free(path);
		return false;
	}

	GtkTreeModel* model = m_view->get_model();
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, LauncherView::COLUMN_LAUNCHER, &m_selected_launcher, -1);
	if (!m_selected_launcher || (m_selected_launcher->get_type() != Launcher::Type))
	{
		m_selected_launcher = NULL;
		m_drag_enabled = false;
		m_view->unset_drag_source();
		m_view->unset_drag_dest();
	}
	else if (!m_drag_enabled)
	{
		m_drag_enabled = true;
		set_reorderable(m_reorderable);
	}

	return false;
}

//-----------------------------------------------------------------------------

gboolean Page::view_button_release_event(GtkWidget*, GdkEvent* event)
{
	GdkEventButton* button_event = reinterpret_cast<GdkEventButton*>(event);
	if (button_event->button != 1)
	{
		return false;
	}

	if (m_launcher_dragged)
	{
		m_window->hide();
		m_launcher_dragged = false;
	}

	return false;
}

//-----------------------------------------------------------------------------

void Page::view_drag_data_get(GtkWidget*, GdkDragContext*, GtkSelectionData* data, guint info, guint)
{
	if ((info != 1) || !m_selected_launcher)
	{
		return;
	}

	gchar* uris[2] = { m_selected_launcher->get_uri(), NULL };
	if (uris[0] != NULL)
	{
		gtk_selection_data_set_uris(data, uris);
		g_free(uris[0]);
	}

	m_launcher_dragged = true;
}

//-----------------------------------------------------------------------------

void Page::view_drag_end(GtkWidget*, GdkDragContext*)
{
	if (m_launcher_dragged)
	{
		m_window->hide();
		m_launcher_dragged = false;
	}
}

//-----------------------------------------------------------------------------

gboolean Page::view_popup_menu_event(GtkWidget*)
{
	GtkTreePath* path = m_view->get_cursor();
	if (!path)
	{
		return false;
	}

	create_context_menu(path, NULL);

	return true;
}

//-----------------------------------------------------------------------------

void Page::create_context_menu(GtkTreePath* path, GdkEvent* event)
{
	// Get selected launcher
	GtkTreeModel* model = m_view->get_model();
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, LauncherView::COLUMN_LAUNCHER, &m_selected_launcher, -1);
	if (!m_selected_launcher || (m_selected_launcher->get_type() != Launcher::Type))
	{
		m_selected_launcher = NULL;
		gtk_tree_path_free(path);
		return;
	}

	// Create context menu
	GtkWidget* menu = gtk_menu_new();
	g_signal_connect_slot(menu, "selection-done", &Page::destroy_context_menu, this);

	// Add menu items
	GtkWidget* menuitem = gtk_menu_item_new_with_label(m_selected_launcher->get_display_name());
	gtk_widget_set_sensitive(menuitem, false);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	const std::vector<DesktopAction*> actions = m_selected_launcher->get_actions();
	if (!actions.empty())
	{
		for (std::vector<DesktopAction*>::size_type i = 0, end = actions.size(); i < end; ++i)
		{
			DesktopAction* action = actions[i];
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
			menuitem = gtk_image_menu_item_new_with_label(action->get_name());
			GtkWidget* image = gtk_image_new_from_icon_name(action->get_icon(), GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
G_GNUC_END_IGNORE_DEPRECATIONS
			g_signal_connect_slot(menuitem, "activate", &Page::launcher_action_activated, this, action);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}

		menuitem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	if (!m_window->get_favorites()->contains(m_selected_launcher))
	{
		menuitem = gtk_image_menu_item_new_with_label(_("Add to Favorites"));
		GtkWidget* image = gtk_image_new_from_icon_name("bookmark-new", GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
		g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &Page::add_selected_to_favorites, this);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	else
	{
		menuitem = gtk_image_menu_item_new_with_label(_("Remove From Favorites"));
		GtkWidget* image = gtk_image_new_from_icon_name("list-remove", GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
		g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &Page::remove_selected_from_favorites, this);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
G_GNUC_END_IGNORE_DEPRECATIONS

	menuitem = gtk_menu_item_new_with_label(_("Add to Desktop"));
	g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &Page::add_selected_to_desktop, this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Add to Panel"));
	g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &Page::add_selected_to_panel, this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Edit Application..."));
	g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &Page::edit_selected, this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	extend_context_menu(menu);

	gtk_widget_show_all(menu);

	// Show context menu
	gtk_menu_attach_to_widget(GTK_MENU(menu), m_view->get_widget(), NULL);
	gtk_menu_popup_at_pointer(GTK_MENU(menu), event);

	// Keep selection
	m_view->select_path(path);
	gtk_tree_path_free(path);
}

//-----------------------------------------------------------------------------

void Page::destroy_context_menu(GtkMenuShell* menu)
{
	m_selected_launcher = NULL;

	gtk_widget_destroy(GTK_WIDGET(menu));

	m_window->on_context_menu_destroyed();
}

//-----------------------------------------------------------------------------

void Page::extend_context_menu(GtkWidget*)
{
}

//-----------------------------------------------------------------------------

void Page::add_selected_to_desktop()
{
	// Fetch desktop folder
	const gchar* desktop_path = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
	GFile* desktop_folder = g_file_new_for_path(desktop_path);

	// Fetch launcher source
	g_assert(m_selected_launcher != NULL);
	GFile* source_file = m_selected_launcher->get_file();

	// Fetch launcher destination
	char* basename = g_file_get_basename(source_file);
	GFile* destination_file = g_file_get_child(desktop_folder, basename);
	g_free(basename);

	// Copy launcher to desktop folder
	GError* error = NULL;
	if (g_file_copy(source_file, destination_file, G_FILE_COPY_NONE, NULL, NULL, NULL, &error))
	{
		// Make launcher executable
		gchar* path = g_file_get_path(destination_file);
		g_chmod(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		g_free(path);
	}
	else
	{
		xfce_dialog_show_error(NULL, error, _("Unable to add launcher to desktop."));
		g_error_free(error);
	}

	g_object_unref(destination_file);
	g_object_unref(source_file);
	g_object_unref(desktop_folder);
}

//-----------------------------------------------------------------------------

void Page::add_selected_to_panel()
{
	// Connect to Xfce panel through D-Bus
	GError* error = NULL;
	GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.xfce.Panel",
			"/org/xfce/Panel",
			"org.xfce.Panel",
			NULL,
			&error);
	if (proxy)
	{
		// Fetch launcher desktop ID
		g_assert(m_selected_launcher != NULL);
		const gchar* parameters[] = { m_selected_launcher->get_desktop_id(), NULL };

		// Tell panel to add item
		if (!g_dbus_proxy_call_sync(proxy,
				"AddNewItem",
				g_variant_new("(s^as)", "launcher", parameters),
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				&error))
		{
			xfce_dialog_show_error(NULL, error, _("Unable to add launcher to panel."));
			g_error_free(error);
		}

		// Disconnect from D-Bus
		g_object_unref(proxy);
	}
	else
	{
		xfce_dialog_show_error(NULL, error, _("Unable to add launcher to panel."));
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------

void Page::add_selected_to_favorites()
{
	g_assert(m_selected_launcher != NULL);
	m_window->get_favorites()->add(m_selected_launcher);
}

//-----------------------------------------------------------------------------

void Page::edit_selected()
{
	g_assert(m_selected_launcher != NULL);

	m_window->hide();

	GError* error = NULL;
	gchar* uri = m_selected_launcher->get_uri();
	gchar* quoted_uri = g_shell_quote(uri);
	gchar* command = g_strconcat("exo-desktop-item-edit ", quoted_uri, NULL);
	g_free(uri);
	g_free(quoted_uri);
	if (g_spawn_command_line_async(command, &error) == false)
	{
		xfce_dialog_show_error(NULL, error, _("Unable to edit launcher."));
		g_error_free(error);
	}
	g_free(command);
}

//-----------------------------------------------------------------------------

void Page::remove_selected_from_favorites()
{
	g_assert(m_selected_launcher != NULL);
	m_window->get_favorites()->remove(m_selected_launcher);
}

//-----------------------------------------------------------------------------
