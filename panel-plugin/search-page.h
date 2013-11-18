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

#include "launcher.h"
#include "page.h"
#include "query.h"

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

private:
	Query m_query;
	std::vector<Launcher*> m_launchers;

	class Match
	{
	public:
		Match(Launcher* launcher = NULL) :
			m_launcher(launcher),
			m_relevancy(G_MAXINT)
		{
		}

		Launcher* launcher() const
		{
			return m_launcher;
		}

		bool operator<(const Match& match) const
		{
			return m_relevancy < match.m_relevancy;
		}

		void update(const Query& query)
		{
			g_assert(m_launcher != NULL);
			m_relevancy = m_launcher->search(query);
		}

		static bool invalid(const Match& match)
		{
			return match.m_relevancy == G_MAXINT;
		}

	private:
		Launcher* m_launcher;
		int m_relevancy;
	};
	std::vector<Match> m_matches;


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
