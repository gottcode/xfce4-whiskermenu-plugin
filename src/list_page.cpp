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


#include "list_page.hpp"

#include "launcher.hpp"
#include "launcher_model.hpp"
#include "launcher_view.hpp"

#include <algorithm>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static std::vector<std::string> read_list(XfceRc* rc, const gchar* key, std::vector<std::string> default_list)
{
	if (!rc || !xfce_rc_has_entry(rc, key))
	{
		return default_list;
	}

	std::vector<std::string> list;
	gchar** values = xfce_rc_read_list_entry(rc, key, ",");
	for (size_t i = 0; values[i] != NULL; ++i)
	{
		list.push_back(values[i]);
	}
	g_strfreev(values);
	return list;
}

//-----------------------------------------------------------------------------

ListPage::ListPage(XfceRc* rc, const gchar* key, const std::vector<std::string>& default_desktop_ids, Menu* menu) :
	Page(menu),
	m_key(key),
	m_desktop_ids(read_list(rc, key, default_desktop_ids))
{
}

//-----------------------------------------------------------------------------

ListPage::~ListPage()
{
	unset_menu_items();
}

//-----------------------------------------------------------------------------

bool ListPage::contains(Launcher* launcher) const
{
	std::string desktop_id = garcon_menu_item_get_desktop_id(launcher->get_item());
	return std::find(m_desktop_ids.begin(), m_desktop_ids.end(), desktop_id) != m_desktop_ids.end();
}

//-----------------------------------------------------------------------------

void ListPage::remove(Launcher* launcher)
{
	LauncherModel model(GTK_LIST_STORE(get_view()->get_model()));
	model.remove_item(launcher);
}

//-----------------------------------------------------------------------------

void ListPage::save(XfceRc* settings)
{
	// Save list
	std::string desktop_ids;
	for (const auto& desktop_id : m_desktop_ids)
	{
		if (!desktop_ids.empty())
		{
			desktop_ids += ",";
		}
		desktop_ids += desktop_id;
	}
	xfce_rc_write_entry(settings, m_key, desktop_ids.c_str());
}

//-----------------------------------------------------------------------------

void ListPage::set_menu_items(const std::map<std::string, Launcher*>& items)
{
	// Create new model for treeview
	LauncherModel model;

	// Fetch menu items or remove them from list if missing
	for (auto i = m_desktop_ids.begin(); i != m_desktop_ids.end(); ++i)
	{
		auto item = items.find(*i);
		if (item != items.end())
		{
			model.append_item(item->second);
		}
		else
		{
			i = m_desktop_ids.erase(i);
			--i;
		}
	}

	// Replace treeview contents
	get_view()->set_model(model.get_model());
	g_signal_connect(get_view()->get_model(), "row-changed", SLOT_CALLBACK(ListPage::on_row_changed), this);
	g_signal_connect(get_view()->get_model(), "row-inserted", SLOT_CALLBACK(ListPage::on_row_inserted), this);
	g_signal_connect(get_view()->get_model(), "row-deleted", SLOT_CALLBACK(ListPage::on_row_deleted), this);
}

//-----------------------------------------------------------------------------

void ListPage::unset_menu_items()
{
	// Clear treeview
	get_view()->unset_model();
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
	gtk_tree_model_get(model, iter, LauncherModel::COLUMN_LAUNCHER, &launcher, -1);
	if (launcher)
	{
		m_desktop_ids[pos] = garcon_menu_item_get_desktop_id(launcher->get_item());
	}
}

//-----------------------------------------------------------------------------

void ListPage::on_row_inserted(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter)
{
	size_t pos = gtk_tree_path_get_indices(path)[0];

	std::string desktop_id;
	Launcher* launcher;
	gtk_tree_model_get(model, iter, LauncherModel::COLUMN_LAUNCHER, &launcher, -1);
	if (launcher)
	{
		desktop_id = garcon_menu_item_get_desktop_id(launcher->get_item());
	}

	if (pos >= m_desktop_ids.size())
	{
		m_desktop_ids.push_back(desktop_id);
	}
	else if (m_desktop_ids.at(pos) != desktop_id)
	{
		m_desktop_ids.insert(m_desktop_ids.begin() + pos, desktop_id);
	}
}

//-----------------------------------------------------------------------------

void ListPage::on_row_deleted(GtkTreeModel*, GtkTreePath* path)
{
	size_t pos = gtk_tree_path_get_indices(path)[0];
	if (pos < m_desktop_ids.size())
	{
		m_desktop_ids.erase(m_desktop_ids.begin() + pos);
	}
}

//-----------------------------------------------------------------------------
