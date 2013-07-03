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


#ifndef WHISKERMENU_LAUNCHER_MODEL_HPP
#define WHISKERMENU_LAUNCHER_MODEL_HPP

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class Launcher;

class LauncherModel
{
public:
	LauncherModel();
	explicit LauncherModel(GtkListStore* model);
	~LauncherModel();

	GtkTreeModel* get_model() const
	{
		return GTK_TREE_MODEL(m_model);
	}

	void append_item(Launcher* launcher)
	{
		insert_item(launcher, INT_MAX);
	}

	void insert_item(Launcher* launcher, int position);

	void prepend_item(Launcher* launcher)
	{
		insert_item(launcher, 0);
	}

	void remove_item(Launcher* launcher);
	void remove_first_item();
	void remove_last_item();

	enum Columns
	{
		COLUMN_ICON = 0,
		COLUMN_TEXT,
		COLUMN_LAUNCHER,
		N_COLUMNS
	};

private:
	LauncherModel(const LauncherModel& model);
	LauncherModel& operator=(const LauncherModel& model);

private:
	GtkListStore* m_model;
};

}

#endif // WHISKERMENU_LAUNCHER_MODEL_HPP
