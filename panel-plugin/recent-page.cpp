/*
 * Copyright (C) 2013, 2015 Graeme Gott <graeme@gottcode.org>
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

#include "recent-page.h"

#include "applications-page.h"
#include "launcher.h"
#include "launcher-view.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

#include <algorithm>

#include <glib/gi18n-lib.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

RecentPage::RecentPage(Window* window) :
	Page(window)
{
	// Prevent going over max
	if (wm_settings->recent.size() > wm_settings->recent_items_max)
	{
		wm_settings->recent.erase(wm_settings->recent.begin() + wm_settings->recent_items_max, wm_settings->recent.end());
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

RecentPage::~RecentPage()
{
	unset_menu_items();
}

//-----------------------------------------------------------------------------

void RecentPage::add(Launcher* launcher)
{
	if (!wm_settings->recent_items_max || !launcher)
	{
		return;
	}
	launcher->set_flag(Launcher::RecentFlag, true);

	std::string desktop_id = launcher->get_desktop_id();
	if (!wm_settings->recent.empty())
	{
		std::vector<std::string>::iterator i = std::find(wm_settings->recent.begin(), wm_settings->recent.end(), desktop_id);

		// Skip if already first launcher
		if (i == wm_settings->recent.begin())
		{
			return;
		}
		// Move to front if already in list
		else if (i != wm_settings->recent.end())
		{
			GtkTreeModel* model = get_view()->get_model();
			GtkTreeIter iter;
			gtk_tree_model_iter_nth_child(model, &iter, NULL, std::distance(wm_settings->recent.begin(), i));
			gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, NULL);
			wm_settings->recent.erase(i);
			wm_settings->recent.insert(wm_settings->recent.begin(), desktop_id);
			wm_settings->set_modified();
			return;
		}
	}

	// Prepend to list of items
	GtkListStore* store = GTK_LIST_STORE(get_view()->get_model());
	gtk_list_store_insert_with_values(
			store, NULL, 0,
			LauncherView::COLUMN_ICON, launcher->get_icon(),
			LauncherView::COLUMN_TEXT, launcher->get_text(),
			LauncherView::COLUMN_TOOLTIP, launcher->get_tooltip(),
			LauncherView::COLUMN_LAUNCHER, launcher,
			-1);
	wm_settings->recent.insert(wm_settings->recent.begin(), desktop_id);
	wm_settings->set_modified();

	// Prevent going over max
	enforce_item_count();
}

//-----------------------------------------------------------------------------

void RecentPage::enforce_item_count()
{
	if (wm_settings->recent_items_max >= wm_settings->recent.size())
	{
		return;
	}

	GtkListStore* store = GTK_LIST_STORE(get_view()->get_model());
	for (ssize_t i = wm_settings->recent.size() - 1, end = wm_settings->recent_items_max; i >= end; --i)
	{
		Launcher* launcher = get_window()->get_applications()->get_application(wm_settings->recent[i]);
		if (launcher)
		{
			launcher->set_flag(Launcher::RecentFlag, false);
		}

		GtkTreeIter iter;
		if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, i))
		{
			gtk_list_store_remove(store, &iter);
		}
	}

	wm_settings->recent.erase(wm_settings->recent.begin() + wm_settings->recent_items_max, wm_settings->recent.end());
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void RecentPage::flag_items(bool enabled)
{
	for (std::vector<std::string>::size_type i = 0, end = wm_settings->recent.size(); i < end; ++i)
	{
		Launcher* launcher = get_window()->get_applications()->get_application(wm_settings->recent[i]);
		if (launcher)
		{
			launcher->set_flag(Launcher::RecentFlag, enabled);
		}
	}
}

//-----------------------------------------------------------------------------

void RecentPage::set_menu_items()
{
	GtkTreeModel* model = get_window()->get_applications()->create_launcher_model(wm_settings->recent);
	get_view()->set_model(model);
	g_object_unref(model);

	flag_items(true);
}

//-----------------------------------------------------------------------------

void RecentPage::unset_menu_items()
{
	// Clear treeview
	get_view()->unset_model();
}

//-----------------------------------------------------------------------------

void RecentPage::extend_context_menu(GtkWidget* menu)
{
	GtkWidget* menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label(_("Clear Recently Used"));
	GtkWidget* image = gtk_image_new_from_icon_name("edit-clear", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &RecentPage::clear_menu, this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

//-----------------------------------------------------------------------------

void RecentPage::clear_menu()
{
	flag_items(false);

	gtk_list_store_clear(GTK_LIST_STORE(get_view()->get_model()));
	wm_settings->recent.clear();
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------
