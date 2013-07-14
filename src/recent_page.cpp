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


#include "recent_page.hpp"

#include "launcher.hpp"
#include "launcher_model.hpp"
#include "launcher_view.hpp"
#include "menu.hpp"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

RecentPage::RecentPage(XfceRc* settings, Menu* menu) :
	ListPage(settings, "recent", std::vector<std::string>(), menu),
	m_max_items(10)
{
	// Prevent going over max
	if (size() > m_max_items)
	{
		std::vector<std::string> desktop_ids = get_desktop_ids();
		desktop_ids.erase(desktop_ids.begin() + m_max_items, desktop_ids.end());
		set_desktop_ids(desktop_ids);
	}
}

//-----------------------------------------------------------------------------

void RecentPage::add(Launcher* launcher)
{
	if (!launcher)
	{
		return;
	}

	// Remove item if already in list
	remove(launcher);

	// Prepend to list of items
	LauncherModel model(GTK_LIST_STORE(get_view()->get_model()));
	model.prepend_item(launcher);

	// Prevent going over max
	while (size() > m_max_items)
	{
		model.remove_last_item();
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
	g_signal_connect(menuitem, "activate", G_CALLBACK(RecentPage::clear_menu_slot), this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

//-----------------------------------------------------------------------------

void RecentPage::clear_menu()
{
	LauncherModel model(GTK_LIST_STORE(get_view()->get_model()));
	for (size_t i = 0, count = size(); i < count; ++i)
	{
		model.remove_first_item();
	}
	get_menu()->set_modified();
}

//-----------------------------------------------------------------------------
