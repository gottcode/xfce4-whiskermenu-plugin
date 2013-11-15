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

#ifndef WHISKERMENU_SEARCH_PAGE_H
#define WHISKERMENU_SEARCH_PAGE_H

#include "page.h"
#include "query.h"

#include <map>
#include <string>
#include <vector>

namespace WhiskerMenu
{

class SearchPage : public Page
{
public:
	explicit SearchPage(Window* window);
	~SearchPage();

	void set_filter(const gchar* filter);
	void set_menu_items(GtkTreeModel* model);
	void unset_menu_items();

private:
	void clear_search(GtkEntry* entry, GtkEntryIconPosition icon_pos);
	bool search_entry_key_press(GtkWidget* widget, GdkEventKey* event);
	static gint on_sort(GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, SearchPage* page);
	static gboolean on_filter(GtkTreeModel* model, GtkTreeIter* iter, SearchPage* page);
	void unset_model();

private:
	Query m_query;
	GtkTreeModelFilter* m_filter_model;
	GtkTreeModelSort* m_sort_model;
	std::vector<Launcher*> m_launchers;
	std::map<std::string, std::map<Launcher*, int> > m_results;
	const std::map<Launcher*, int>* m_current_results;


private:
	static void clear_search_slot(GtkEntry* entry, GtkEntryIconPosition icon_pos, GdkEvent*, SearchPage* obj)
	{
		obj->clear_search(entry, icon_pos);
	}

	static gboolean search_entry_key_press_slot(GtkWidget* widget, GdkEventKey* event, SearchPage* obj)
	{
		return obj->search_entry_key_press(widget, event);
	}
};

}

#endif // WHISKERMENU_SEARCH_PAGE_H
