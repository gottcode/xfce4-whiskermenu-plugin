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

#ifndef WHISKERMENU_FAVORITES_PAGE_H
#define WHISKERMENU_FAVORITES_PAGE_H

#include "list-page.h"

namespace WhiskerMenu
{

class Launcher;

class FavoritesPage : public ListPage
{
public:
	FavoritesPage(XfceRc* settings, Window* window);

	void add(Launcher* launcher);

	static bool get_remember_favorites();
	static void set_remember_favorites(bool remember);

private:
	void extend_context_menu(GtkWidget* menu);
	bool remember_launcher(Launcher* launcher);
	void sort(std::vector<Launcher*>& items) const;
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

#endif // WHISKERMENU_FAVORITES_PAGE_H
