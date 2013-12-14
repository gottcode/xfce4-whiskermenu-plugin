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

#include "search-page.h"

#include "launcher.h"
#include "launcher-view.h"
#include "search-action.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

#include <algorithm>

#include <gdk/gdkkeysyms.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

SearchPage::SearchPage(Window* window) :
	Page(window)
{
	get_view()->set_selection_mode(GTK_SELECTION_BROWSE);

	g_signal_connect_slot(window->get_search_entry(), "icon-release", &SearchPage::clear_search, this);
	g_signal_connect_slot(window->get_search_entry(), "key-press-event", &SearchPage::search_entry_key_press, this);
}

//-----------------------------------------------------------------------------

SearchPage::~SearchPage()
{
	unset_menu_items();
}

//-----------------------------------------------------------------------------

void SearchPage::set_filter(const gchar* filter)
{
	// Clear search results for empty filter
	if (!filter)
	{
		m_query.clear();
		m_matches.clear();
		return;
	}

	// Make sure this is a new search
	std::string query(filter);
	if (m_query.raw_query() == query)
	{
		return;
	}

	// Reset search results if new search does not start with previous search
	if (m_query.raw_query().empty() || !g_str_has_prefix(filter, m_query.raw_query().c_str()))
	{
		m_matches.clear();
		m_matches.push_back(&m_run_action);
		for (std::vector<Launcher*>::size_type i = 0, end = m_launchers.size(); i < end; ++i)
		{
			m_matches.push_back(m_launchers[i]);
		}
	}
	else if (std::find(m_matches.begin(), m_matches.end(), &m_run_action) == m_matches.end())
	{
		m_matches.insert(m_matches.begin(), &m_run_action);
	}
	m_query.set(query);

	// Create search results
	for (std::vector<Match>::size_type i = 0, end = m_matches.size(); i < end; ++i)
	{
		m_matches[i].update(m_query);
	}
	m_matches.erase(std::remove_if(m_matches.begin(), m_matches.end(), &Match::invalid), m_matches.end());
	std::stable_sort(m_matches.begin(), m_matches.end());

	// Show search results
	GtkListStore* store = gtk_list_store_new(
			LauncherView::N_COLUMNS,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_POINTER);
	SearchAction* action;
	for (std::vector<SearchAction*>::size_type i = 0, end = wm_settings->search_actions.size(); i < end; ++i)
	{
		action = wm_settings->search_actions[i];
		if (action->search(m_query) != G_MAXINT)
		{
			gtk_list_store_insert_with_values(
					store, NULL, G_MAXINT,
					LauncherView::COLUMN_ICON, action->get_icon(),
					LauncherView::COLUMN_TEXT, action->get_text(),
					LauncherView::COLUMN_LAUNCHER, action,
					-1);
		}
	}
	Element* element;
	for (std::vector<Match>::size_type i = 0, end = m_matches.size(); i < end; ++i)
	{
		element = m_matches[i].element();
		gtk_list_store_insert_with_values(
				store, NULL, G_MAXINT,
				LauncherView::COLUMN_ICON, element->get_icon(),
				LauncherView::COLUMN_TEXT, element->get_text(),
				LauncherView::COLUMN_LAUNCHER, element,
				-1);
	}
	get_view()->set_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	// Find first result
	GtkTreeIter iter;
	GtkTreePath* path = gtk_tree_path_new_first();
	bool found = gtk_tree_model_get_iter(get_view()->get_model(), &iter, path);

	// Scroll to and select first result
	if (found)
	{
		get_view()->select_path(path);
		get_view()->scroll_to_path(path);
	}
	gtk_tree_path_free(path);
}

//-----------------------------------------------------------------------------

void SearchPage::set_menu_items(GtkTreeModel* model)
{
	// loop over every single item in model
	GtkTreeIter iter;
	bool valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid)
	{
		Launcher* launcher = NULL;
		gtk_tree_model_get(model, &iter, LauncherView::COLUMN_LAUNCHER, &launcher, -1);
		if (launcher)
		{
			m_launchers.push_back(launcher);
		}
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	get_view()->unset_model();

	m_matches.clear();
	m_matches.reserve(m_launchers.size() + 1);
}

//-----------------------------------------------------------------------------

void SearchPage::unset_menu_items()
{
	m_launchers.clear();
	m_matches.clear();
	get_view()->unset_model();
}

//-----------------------------------------------------------------------------

void SearchPage::clear_search(GtkEntry* entry, GtkEntryIconPosition icon_pos, GdkEvent*)
{
	if (icon_pos == GTK_ENTRY_ICON_SECONDARY)
	{
		gtk_entry_set_text(entry, "");
	}
}

//-----------------------------------------------------------------------------

gboolean SearchPage::search_entry_key_press(GtkWidget* widget, GdkEvent* event)
{
	GdkEventKey* key_event = reinterpret_cast<GdkEventKey*>(event);
	if (key_event->keyval == GDK_Escape)
	{
		GtkEntry* entry = GTK_ENTRY(widget);
		const gchar* text = gtk_entry_get_text(entry);
		if ((text != NULL) && (*text != '\0'))
		{
			gtk_entry_set_text(entry, "");
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (key_event->keyval == GDK_Return)
	{
		GtkTreePath* path = get_view()->get_selected_path();
		if (path)
		{
			get_view()->activate_path(path);
			gtk_tree_path_free(path);
		}
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
