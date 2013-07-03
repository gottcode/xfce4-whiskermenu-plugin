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
	m_sort_model(NULL)
{
	get_view()->set_selection_mode(GTK_SELECTION_BROWSE);

	g_signal_connect(menu->get_search_entry(), "icon-release", SLOT_CALLBACK(SearchPage::clear_search), this);
	g_signal_connect(menu->get_search_entry(), "key-press-event", SLOT_CALLBACK(SearchPage::search_entry_key_press), this);
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

	// Create search results
	for (std::vector<Launcher*>::iterator i = m_launchers.begin(), end = m_launchers.end(); i != end; ++i)
	{
		(*i)->search(m_query);
	}

	// Apply filter
	GtkTreeModel* filter_model = gtk_tree_model_sort_get_model(m_sort_model);
	get_view()->unset_model();
	unset_search_model();

	refilter();

	set_search_model(filter_model);

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
		m_launchers.push_back(launcher);
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	unset_search_model();
	set_model(model);
	set_search_model(get_view()->get_model());
}

//-----------------------------------------------------------------------------

void SearchPage::unset_menu_items()
{
	m_launchers.clear();
	unset_search_model();
	unset_model();
}

//-----------------------------------------------------------------------------

bool SearchPage::on_filter(GtkTreeModel* model, GtkTreeIter* iter)
{
	if (m_query.empty())
	{
		return false;
	}

	// Check if launcher search string contains text
	Launcher* launcher = NULL;
	gtk_tree_model_get(model, iter, LauncherModel::COLUMN_LAUNCHER, &launcher, -1);
	return launcher->get_search_results(m_query) != UINT_MAX;
}

//-----------------------------------------------------------------------------

gint SearchPage::on_sort(GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, SearchPage* page)
{
	Launcher* launcher_a = NULL;
	gtk_tree_model_get(model, a, LauncherModel::COLUMN_LAUNCHER, &launcher_a, -1);

	Launcher* launcher_b = NULL;
	gtk_tree_model_get(model, b, LauncherModel::COLUMN_LAUNCHER, &launcher_b, -1);

	return launcher_a->get_search_results(page->m_query) - launcher_b->get_search_results(page->m_query);
}

//-----------------------------------------------------------------------------

void SearchPage::set_search_model(GtkTreeModel* child_model)
{
	m_sort_model = GTK_TREE_MODEL_SORT(gtk_tree_model_sort_new_with_model(child_model));
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(m_sort_model), (GtkTreeIterCompareFunc)&SearchPage::on_sort, this, NULL);
	get_view()->set_model(GTK_TREE_MODEL(m_sort_model));
}

//-----------------------------------------------------------------------------

void SearchPage::unset_search_model()
{
	if (m_sort_model)
	{
		g_object_unref(m_sort_model);
		m_sort_model = NULL;
	}
}

//-----------------------------------------------------------------------------

void SearchPage::clear_search(GtkEntry* entry, GtkEntryIconPosition icon_pos, GdkEvent*)
{
	if (icon_pos == GTK_ENTRY_ICON_SECONDARY)
	{
		gtk_entry_set_text(GTK_ENTRY(entry), "");
	}
}

//-----------------------------------------------------------------------------

gboolean SearchPage::search_entry_key_press(GtkWidget* widget, GdkEventKey* event)
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
