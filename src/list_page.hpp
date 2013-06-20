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


#ifndef WHISKERMENU_LIST_PAGE_HPP
#define WHISKERMENU_LIST_PAGE_HPP

#include "page.hpp"

#include <string>
#include <unordered_map>
#include <vector>

extern "C"
{
#include <libxfce4util/libxfce4util.h>
}

namespace WhiskerMenu
{

class Launcher;
class LauncherView;

class ListPage : public Page
{
public:
	ListPage(XfceRc* settings, const gchar* key, std::vector<std::string> default_desktop_ids, Menu* menu);
	~ListPage();

	bool contains(Launcher* launcher) const;
	size_t size() const
	{
		return m_desktop_ids.size();
	}

	virtual void add(Launcher* launcher)=0;
	void remove(Launcher* launcher);
	void save(XfceRc* settings);
	void set_menu_items(std::unordered_map<std::string, Launcher*> items);
	void unset_menu_items();

private:
	void on_row_changed(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter);
	void on_row_inserted(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter);
	void on_row_deleted(GtkTreeModel* model, GtkTreePath* path);

private:
	const gchar* m_key;
	std::vector<std::string> m_desktop_ids;
};

}

#endif // WHISKERMENU_LIST_PAGE_HPP
