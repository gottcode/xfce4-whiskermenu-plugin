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


#ifndef WHISKERMENU_SEARCH_PAGE_HPP
#define WHISKERMENU_SEARCH_PAGE_HPP

#include "filter_page.hpp"
#include "slot.hpp"

#include <string>
#include <vector>

namespace WhiskerMenu
{

class LauncherView;

class SearchPage : public FilterPage
{
public:
	SearchPage(Menu* menu);
	~SearchPage();

	void set_filter(const gchar* filter);
	void set_menu_items(GtkTreeModel* model);
	void unset_menu_items();

private:
	SLOT_3(void, SearchPage, clear_search, GtkEntry*, GtkEntryIconPosition, GdkEvent*);
	SLOT_2(gboolean, SearchPage, search_entry_key_press, GtkWidget*, GdkEventKey*);

private:
	bool on_filter(GtkTreeModel* model, GtkTreeIter* iter);
	static gint on_sort(GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, SearchPage* page);
	void set_search_model(GtkTreeModel* child_model);
	void unset_search_model();

private:
	std::string m_filter_string;
	GtkTreeModelSort* m_sort_model;
	std::vector<Launcher*> m_launchers;
};

}

#endif // WHISKERMENU_SEARCH_PAGE_HPP
