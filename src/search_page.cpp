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
#include "slot.hpp"

extern "C"
{
#include <gdk/gdkkeysyms.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

SearchPage::SearchPage(Menu* menu) :
	FilterPage(menu),
	m_filter_matching_path(nullptr)
{
	get_view()->set_selection_mode(GTK_SELECTION_BROWSE);

	g_signal_connect_slot(menu->get_search_entry(), "icon-release", &SearchPage::clear_search, this);
	g_signal_connect_slot(menu->get_search_entry(), "key-press-event", &SearchPage::search_entry_key_press, this);
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
	std::string old_filter_string = m_filter_string;
	if (filter)
	{
		m_filter_string = filter;
	}
	else
	{
		m_filter_string.clear();
	}
	if (m_filter_string == old_filter_string)
	{
		return;
	}

	// Apply filter
	refilter();

	if (m_filter_matching_path)
	{
		// Scroll to and select first result that begins with search string
		GtkTreePath* path = convert_child_path_to_path(m_filter_matching_path);
		gtk_tree_path_free(m_filter_matching_path);
		m_filter_matching_path = nullptr;

		get_view()->select_path(path);
		get_view()->scroll_to_path(path);
		gtk_tree_path_free(path);
	}
	else
	{
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
}

//-----------------------------------------------------------------------------

void SearchPage::set_menu_items(GtkTreeModel* model)
{
	set_model(model);
}

//-----------------------------------------------------------------------------

void SearchPage::unset_menu_items()
{
	unset_model();
}

//-----------------------------------------------------------------------------

bool SearchPage::on_filter(GtkTreeModel* model, GtkTreeIter* iter)
{
	if (m_filter_string.empty())
	{
		return false;
	}

	// Check if launcher search string contains text
	Launcher* launcher = nullptr;
	gtk_tree_model_get(model, iter, LauncherModel::COLUMN_LAUNCHER, &launcher, -1);
	gchar* result = g_strstr_len(launcher->get_search_text(), -1, m_filter_string.c_str());

	// Check if launcher search string starts with text
	if (!m_filter_matching_path && (result == launcher->get_search_text()))
	{
		m_filter_matching_path = gtk_tree_model_get_path(model, iter);
	}

	return result != nullptr;
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
		if ((text != nullptr) && (*text != '\0'))
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
