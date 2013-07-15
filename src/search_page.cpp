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


#include "search_page.hpp"

#include "launcher.hpp"
#include "launcher_model.hpp"
#include "launcher_view.hpp"
#include "menu.hpp"

extern "C"
{
#include <gdk/gdkkeysyms.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

SearchPage::SearchPage(Menu* menu) :
	FilterPage(menu),
	m_sort_model(NULL),
	m_current_results(NULL)
{
	get_view()->set_selection_mode(GTK_SELECTION_BROWSE);

	g_signal_connect(menu->get_search_entry(), "icon-release", G_CALLBACK(SearchPage::clear_search_slot), this);
	g_signal_connect(menu->get_search_entry(), "key-press-event", G_CALLBACK(SearchPage::search_entry_key_press_slot), this);
}

//-----------------------------------------------------------------------------

SearchPage::~SearchPage()
{
	unset_menu_items();
}

//-----------------------------------------------------------------------------

void SearchPage::set_filter(const gchar* filter)
{
	// Store filter string
	std::string query(filter ? filter : "");
	if (m_query.query() == query)
	{
		return;
	}
	m_query.set(query);

	// Find longest previous search that starts query
	const std::map<Launcher*, int>* previous = NULL;
	m_current_results = NULL;
	for (std::map<std::string, std::map<Launcher*, int> >::const_reverse_iterator i = m_results.rbegin(), end = m_results.rend(); i != end; ++i)
	{
		if ( (i->first.length() < query.length())
			&& (query.compare(0, i->first.length(), i->first) == 0) )
		{
			previous = &i->second;
			break;
		}
		else if (i->first == query)
		{
			m_current_results = &i->second;
			break;
		}
	}

	// Create search results
	if (!m_current_results && !m_query.empty())
	{
		std::map<Launcher*, int> results;
		if (previous)
		{
			// Only check launchers that had previous search results
			for (std::map<Launcher*, int>::const_iterator i = previous->begin(), end = previous->end(); i != end; ++i)
			{
				int result = i->first->search(m_query);
				if (result != INT_MAX)
				{
					results.insert(std::make_pair(i->first, result));
				}
			}
		}
		else
		{
			// Check all launchers
			for (std::vector<Launcher*>::const_iterator i = m_launchers.begin(), end = m_launchers.end(); i != end; ++i)
			{
				int result = (*i)->search(m_query);
				if (result != INT_MAX)
				{
					results.insert(std::make_pair(*i, result));
				}
			}
		}
		m_current_results = &m_results.insert(std::make_pair(query, results)).first->second;
	}

	// Show search results
	g_object_freeze_notify(G_OBJECT(get_view()->get_widget()));
	get_view()->unset_model();
	gtk_tree_model_sort_reset_default_sort_func(m_sort_model);

	refilter();

	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(m_sort_model), (GtkTreeIterCompareFunc)&SearchPage::on_sort, this, NULL);
	get_view()->set_model(GTK_TREE_MODEL(m_sort_model));
	g_object_thaw_notify(G_OBJECT(get_view()->get_widget()));

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
		gtk_tree_model_get(model, &iter, LauncherModel::COLUMN_LAUNCHER, &launcher, -1);
		if (launcher)
		{
			m_launchers.push_back(launcher);
		}
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	unset_search_model();
	set_model(model);
	m_sort_model = GTK_TREE_MODEL_SORT(gtk_tree_model_sort_new_with_model(get_view()->get_model()));
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(m_sort_model), (GtkTreeIterCompareFunc)&SearchPage::on_sort, this, NULL);
	get_view()->unset_model();
}

//-----------------------------------------------------------------------------

void SearchPage::unset_menu_items()
{
	m_launchers.clear();
	m_results.clear();
	m_current_results = NULL;
	unset_search_model();
	unset_model();
}

//-----------------------------------------------------------------------------

bool SearchPage::on_filter(GtkTreeModel* model, GtkTreeIter* iter)
{
	if (!m_current_results)
	{
		return false;
	}

	// Check if launcher search string contains text
	Launcher* launcher = NULL;
	gtk_tree_model_get(model, iter, LauncherModel::COLUMN_LAUNCHER, &launcher, -1);
	return launcher && (m_current_results->find(launcher) != m_current_results->end());
}

//-----------------------------------------------------------------------------

gint SearchPage::on_sort(GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, SearchPage* page)
{
	Launcher* launcher_a = NULL;
	gtk_tree_model_get(model, a, LauncherModel::COLUMN_LAUNCHER, &launcher_a, -1);
	g_assert(launcher_a != NULL);
	g_assert(page->m_current_results->find(launcher_a) != page->m_current_results->end());

	Launcher* launcher_b = NULL;
	gtk_tree_model_get(model, b, LauncherModel::COLUMN_LAUNCHER, &launcher_b, -1);
	g_assert(launcher_b != NULL);
	g_assert(page->m_current_results->find(launcher_b) != page->m_current_results->end());

	return page->m_current_results->find(launcher_a)->second - page->m_current_results->find(launcher_b)->second;
}

//-----------------------------------------------------------------------------

void SearchPage::unset_search_model()
{
	if (m_sort_model)
	{
		g_object_unref(m_sort_model);
		m_sort_model = NULL;
	}
	get_view()->unset_model();
}

//-----------------------------------------------------------------------------

void SearchPage::clear_search(GtkEntry* entry, GtkEntryIconPosition icon_pos)
{
	if (icon_pos == GTK_ENTRY_ICON_SECONDARY)
	{
		gtk_entry_set_text(entry, "");
	}
}

//-----------------------------------------------------------------------------

bool SearchPage::search_entry_key_press(GtkWidget* widget, GdkEventKey* event)
{
	if (event->keyval == GDK_Escape)
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
	else if (event->keyval == GDK_Return)
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
