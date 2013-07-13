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


#include "favorites_page.hpp"

#include "applications_page.hpp"
#include "launcher.hpp"
#include "launcher_model.hpp"
#include "launcher_view.hpp"
#include "menu.hpp"

#include <algorithm>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static const std::string default_favorites[4] =
{
	"exo-terminal-emulator.desktop",
	"exo-file-manager.desktop",
	"exo-mail-reader.desktop",
	"exo-web-browser.desktop"
};

//-----------------------------------------------------------------------------

FavoritesPage::FavoritesPage(XfceRc* settings, Menu* menu) :
	ListPage(settings, "favorites",
	std::vector<std::string>(default_favorites, default_favorites + 4),
	menu)
{
	get_view()->set_reorderable(true);
}

//-----------------------------------------------------------------------------

void FavoritesPage::add(Launcher* launcher)
{
	if (!launcher)
	{
		return;
	}

	// Remove item if already in list
	remove(launcher);

	// Add to list of items
	LauncherModel model(GTK_LIST_STORE(get_view()->get_model()));
	model.append_item(launcher);
}

//-----------------------------------------------------------------------------

void FavoritesPage::extend_context_menu(GtkWidget* menu)
{
	GtkWidget* menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label(_("Sort Alphabetically A-Z"));
	GtkWidget* image = gtk_image_new_from_stock(GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect(menuitem, "activate", G_CALLBACK(FavoritesPage::sort_ascending_slot), this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label(_("Sort Alphabetically Z-A"));
	image = gtk_image_new_from_stock(GTK_STOCK_SORT_DESCENDING, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect(menuitem, "activate", G_CALLBACK(FavoritesPage::sort_descending_slot), this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

//-----------------------------------------------------------------------------

void FavoritesPage::sort(std::map<std::string, Launcher*>& items) const
{
	const std::vector<std::string>& desktop_ids = get_desktop_ids();
	for (std::vector<std::string>::const_iterator i = desktop_ids.begin(), end = desktop_ids.end(); i != end; ++i)
	{
		Launcher* launcher = get_menu()->get_applications()->get_application(*i);
		if (!launcher)
		{
			continue;
		}
		gchar* collation_key = g_utf8_collate_key(launcher->get_text(), -1);
		items[collation_key] = launcher;
		g_free(collation_key);
	}
}

//-----------------------------------------------------------------------------

void FavoritesPage::sort_ascending()
{
	std::vector<std::string> desktop_ids;
	std::map<std::string, Launcher*> items;
	sort(items);
	for (std::map<std::string, Launcher*>::const_iterator i = items.begin(), end = items.end(); i != end; ++i)
	{
		desktop_ids.push_back(i->second->get_desktop_id());
	}
	set_desktop_ids(desktop_ids);
}

//-----------------------------------------------------------------------------

void FavoritesPage::sort_descending()
{
	std::vector<std::string> desktop_ids;
	std::map<std::string, Launcher*> items;
	sort(items);
	for (std::map<std::string, Launcher*>::const_reverse_iterator i = items.rbegin(), end = items.rend(); i != end; ++i)
	{
		desktop_ids.push_back(i->second->get_desktop_id());
	}
	set_desktop_ids(desktop_ids);
}

//-----------------------------------------------------------------------------
