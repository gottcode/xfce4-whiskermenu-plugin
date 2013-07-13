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


#ifndef WHISKERMENU_LAUNCHER_VIEW_HPP
#define WHISKERMENU_LAUNCHER_VIEW_HPP

#include "slot.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class LauncherView
{
public:
	LauncherView();
	~LauncherView();

	GtkWidget* get_widget() const
	{
		return GTK_WIDGET(m_view);
	}

	GtkTreePath* get_selected_path() const;
	void activate_path(GtkTreePath* path);

	void scroll_to_path(GtkTreePath* path);
	void select_path(GtkTreePath* path);

	void set_reorderable(bool reorderable);
	void set_selection_mode(GtkSelectionMode mode);

	void unselect_all();

	GtkTreeModel* get_model() const
	{
		return m_model;
	}

	void set_model(GtkTreeModel* model);
	void unset_model();

private:
	SLOT_2(gboolean, LauncherView, on_key_press_event, GtkWidget*, GdkEventKey*);
	SLOT_2(gboolean, LauncherView, on_key_release_event, GtkWidget*, GdkEventKey*);

private:
	GtkTreeModel* m_model;
	GtkTreeView* m_view;
};

}

#endif // WHISKERMENU_LAUNCHER_VIEW_HPP
