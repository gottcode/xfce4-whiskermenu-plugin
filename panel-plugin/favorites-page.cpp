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

#include "favorites-page.h"

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

FavoritesPage::FavoritesPage(Window* window) :
	ListPage(wm_settings->favorites, window)
{
	get_view()->set_reorderable(true);
}

//-----------------------------------------------------------------------------

void FavoritesPage::add(Launcher* launcher)
{
	if (!launcher || contains(launcher))
	{
		return;
	}

	// Append to list of items
	GtkListStore* store = GTK_LIST_STORE(get_view()->get_model());
	gtk_list_store_insert_with_values(
			store, NULL, G_MAXINT,
			LauncherView::COLUMN_ICON, launcher->get_icon(),
			LauncherView::COLUMN_TEXT, launcher->get_text(),
			LauncherView::COLUMN_LAUNCHER, launcher,
			-1);
}

//-----------------------------------------------------------------------------

void FavoritesPage::extend_context_menu(GtkWidget* menu)
{
	GtkWidget* menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label(_("Sort Alphabetically A-Z"));
	GtkWidget* image = gtk_image_new_from_stock(GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &FavoritesPage::sort_ascending, this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label(_("Sort Alphabetically Z-A"));
	image = gtk_image_new_from_stock(GTK_STOCK_SORT_DESCENDING, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_slot<GtkMenuItem*>(menuitem, "activate", &FavoritesPage::sort_descending, this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

//-----------------------------------------------------------------------------

bool FavoritesPage::remember_launcher(Launcher* launcher)
{
	return wm_settings->favorites_in_recent ? true : !contains(launcher);
}

//-----------------------------------------------------------------------------

void FavoritesPage::sort(std::vector<Launcher*>& items) const
{
	for (std::vector<std::string>::const_iterator i = wm_settings->favorites.begin(), end = wm_settings->favorites.end(); i != end; ++i)
	{
		Launcher* launcher = get_window()->get_applications()->get_application(*i);
		if (!launcher)
		{
			continue;
		}
		items.push_back(launcher);
	}
	std::sort(items.begin(), items.end(), &Element::less_than);
}

//-----------------------------------------------------------------------------

void FavoritesPage::sort_ascending()
{
	std::vector<Launcher*> items;
	sort(items);

	std::vector<std::string> desktop_ids;
	for (std::vector<Launcher*>::const_iterator i = items.begin(), end = items.end(); i != end; ++i)
	{
		desktop_ids.push_back((*i)->get_desktop_id());
	}
	set_desktop_ids(desktop_ids);

	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void FavoritesPage::sort_descending()
{
	std::vector<Launcher*> items;
	sort(items);

	std::vector<std::string> desktop_ids;
	for (std::vector<Launcher*>::const_reverse_iterator i = items.rbegin(), end = items.rend(); i != end; ++i)
	{
		desktop_ids.push_back((*i)->get_desktop_id());
	}
	set_desktop_ids(desktop_ids);

	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------
