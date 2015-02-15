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
#include "run-action.h"

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
	void activate_search();
	void clear_search(GtkEntry* entry, GtkEntryIconPosition icon_pos, GdkEvent*);
	gboolean cancel_search(GtkWidget* widget, GdkEvent* event);

private:
	Query m_query;
	std::vector<Launcher*> m_launchers;
	RunAction m_run_action;

	class Match
	{
	public:
		Match(Element* element = NULL) :
			m_element(element),
			m_relevancy(G_MAXINT)
		{
		}

		Element* element() const
		{
			return m_element;
		}

		bool operator<(const Match& match) const
		{
			return m_relevancy < match.m_relevancy;
		}

		bool operator==(const Match& match) const
		{
			return m_element == match.m_element;
		}

		void update(const Query& query)
		{
			g_assert(m_element != NULL);
			m_relevancy = m_element->search(query);
		}

		static bool invalid(const Match& match)
		{
			return match.m_relevancy == G_MAXINT;
		}

	private:
		Element* m_element;
		int m_relevancy;
	};
	std::vector<Match> m_matches;
};

}

#endif // WHISKERMENU_SEARCH_PAGE_H
