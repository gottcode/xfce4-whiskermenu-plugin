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


#ifndef WHISKERMENU_RECENT_PAGE_HPP
#define WHISKERMENU_RECENT_PAGE_HPP

#include "list_page.hpp"

namespace WhiskerMenu
{

class Launcher;
class LauncherView;

class RecentPage : public ListPage
{
public:
	RecentPage(XfceRc* settings, Menu* menu);

	void add(Launcher* launcher);

private:
	void extend_context_menu(GtkWidget* menu);
	void clear_menu();

private:
	size_t m_max_items;


private:
	static void clear_menu_slot(GtkMenuItem*, RecentPage* obj)
	{
		obj->clear_menu();
	}
};

}

#endif // WHISKERMENU_RECENT_PAGE_HPP
