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


#ifndef WHISKERMENU_FAVORITES_PAGE_HPP
#define WHISKERMENU_FAVORITES_PAGE_HPP

#include "list_page.hpp"

namespace WhiskerMenu
{

class Launcher;

class FavoritesPage : public ListPage
{
public:
	FavoritesPage(XfceRc* settings, Menu* menu);

	void add(Launcher* launcher);

private:
	void extend_context_menu(GtkWidget* menu);
	void sort(std::map<std::string, Launcher*>& items) const;
	void sort_ascending();
	void sort_descending();


private:
	static void sort_ascending_slot(GtkMenuItem*, FavoritesPage* obj)
	{
		obj->sort_ascending();
	}

	static void sort_descending_slot(GtkMenuItem*, FavoritesPage* obj)
	{
		obj->sort_descending();
	}
};

}

#endif // WHISKERMENU_FAVORITES_PAGE_HPP
