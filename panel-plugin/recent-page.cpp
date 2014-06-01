/*
 * Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
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

#include "launcher.h"
#include "launcher-view.h"
#include "settings.h"
#include "slot.h"

#include <glib/gi18n-lib.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

RecentPage::RecentPage(Window* window) :
	ListPage(wm_settings->recent, window)
{
	// Prevent going over max
	if (wm_settings->recent.size() > wm_settings->recent_items_max)
	{
		wm_settings->recent.erase(wm_settings->recent.begin() + wm_settings->recent_items_max, wm_settings->recent.end());
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

void RecentPage::add(Launcher* launcher)
{
	if (!launcher)
	{
		return;
	}

	// Skip if already first launcher
	if (!wm_settings->recent.empty() && (wm_settings->recent.front() == launcher->get_desktop_id()))
	{
		return;
	}

	// Remove item if already in list
	remove(launcher);

	// Prepend to list of items
	GtkListStore* store = GTK_LIST_STORE(get_view()->get_model());
	gtk_list_store_insert_with_values(
			store, NULL, 0,
			LauncherView::COLUMN_ICON, launcher->get_icon(),
			LauncherView::COLUMN_TEXT, launcher->get_text(),
			LauncherView::COLUMN_LAUNCHER, launcher,
			-1);

	// Prevent going over max
	while (wm_settings->recent.size() > wm_settings->recent_items_max)
	{
		GtkTreeIter iter;
		if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, wm_settings->recent.size() - 1))
		{
			gtk_list_store_remove(store, &iter);
		}
	}
}

//-----------------------------------------------------------------------------

void RecentPage::extend_context_menu(GtkWidget* menu)
{
	GtkWidget* menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label(_("Clear Recently Used"));
	GtkWidget* image = gtk_image_new_from_stock(GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &RecentPage::clear_menu, this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

//-----------------------------------------------------------------------------

void RecentPage::clear_menu()
{
	gtk_list_store_clear(GTK_LIST_STORE(get_view()->get_model()));
}

//-----------------------------------------------------------------------------
