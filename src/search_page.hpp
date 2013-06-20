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

#include <string>

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
	bool on_filter(GtkTreeModel* model, GtkTreeIter* iter);
	void clear_search(GtkEntry* entry, GtkEntryIconPosition icon_pos, GdkEvent*);
	gboolean search_entry_key_press(GtkWidget* widget, GdkEventKey* event);

private:
	std::string m_filter_string;
	GtkTreePath* m_filter_matching_path;
};

}

#endif // WHISKERMENU_SEARCH_PAGE_HPP
