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

#include "list-page.h"

#include "applications-page.h"
#include "launcher.h"
#include "launcher-view.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

ListPage::ListPage(std::vector<std::string>& desktop_ids, Window* window) :
	Page(window),
	m_desktop_ids(desktop_ids)
{
}

//-----------------------------------------------------------------------------

ListPage::~ListPage()
{
	unset_menu_items();
}

//-----------------------------------------------------------------------------

void ListPage::set_menu_items()
{
	GtkTreeModel* model = get_window()->get_applications()->create_launcher_model(m_desktop_ids);
	get_view()->set_model(model);
	g_signal_connect_slot(model, "row-changed", &ListPage::on_row_changed, this);
	g_signal_connect_slot(model, "row-inserted", &ListPage::on_row_inserted, this);
	g_signal_connect_slot(model, "row-deleted", &ListPage::on_row_deleted, this);
	g_object_unref(model);
}

//-----------------------------------------------------------------------------

void ListPage::unset_menu_items()
{
	// Clear treeview
	get_view()->unset_model();
}

//-----------------------------------------------------------------------------

void ListPage::set_desktop_ids(const std::vector<std::string>& desktop_ids)
{
	m_desktop_ids = desktop_ids;
	if (get_view()->get_model())
	{
		set_menu_items();
	}
}

//-----------------------------------------------------------------------------

void ListPage::on_row_changed(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter)
{
	size_t pos = gtk_tree_path_get_indices(path)[0];
	if (pos >= m_desktop_ids.size())
	{
		return;
	}

	Launcher* launcher;
	gtk_tree_model_get(model, iter, LauncherView::COLUMN_LAUNCHER, &launcher, -1);
	if (launcher)
	{
		g_assert(launcher->get_type() == Launcher::Type);
		m_desktop_ids[pos] = launcher->get_desktop_id();
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

void ListPage::on_row_inserted(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter)
{
	size_t pos = gtk_tree_path_get_indices(path)[0];

	std::string desktop_id;
	Launcher* launcher;
	gtk_tree_model_get(model, iter, LauncherView::COLUMN_LAUNCHER, &launcher, -1);
	if (launcher)
	{
		g_assert(launcher->get_type() == Launcher::Type);
		desktop_id = launcher->get_desktop_id();
	}

	if (pos >= m_desktop_ids.size())
	{
		m_desktop_ids.push_back(desktop_id);
		wm_settings->set_modified();
	}
	else if (m_desktop_ids.at(pos) != desktop_id)
	{
		m_desktop_ids.insert(m_desktop_ids.begin() + pos, desktop_id);
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

void ListPage::on_row_deleted(GtkTreeModel*, GtkTreePath* path)
{
	size_t pos = gtk_tree_path_get_indices(path)[0];
	if (pos < m_desktop_ids.size())
	{
		m_desktop_ids.erase(m_desktop_ids.begin() + pos);
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------
