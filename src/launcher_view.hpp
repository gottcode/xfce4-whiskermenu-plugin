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

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class IconSize;

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
	void set_cursor(GtkTreePath* path);

	void set_reorderable(bool reorderable);
	void set_selection_mode(GtkSelectionMode mode);

	void unselect_all();

	GtkTreeModel* get_model() const
	{
		return m_model;
	}

	void set_model(GtkTreeModel* model);
	void unset_model();

	void reload_icon_size();

	static int get_icon_size();
	static void set_icon_size(const int size);

private:
	void create_column();
	bool on_key_press_event(GdkEventKey* event);
	bool on_key_release_event(GdkEventKey* event);

private:
	GtkTreeModel* m_model;
	GtkTreeView* m_view;
	GtkTreeViewColumn* m_column;
	GtkCellRenderer* m_icon_renderer;


private:
	static gboolean on_key_press_event_slot(GtkWidget*, GdkEventKey* event, LauncherView* obj)
	{
		return obj->on_key_press_event(event);
	}

	static gboolean on_key_release_event_slot(GtkWidget*, GdkEventKey* event, LauncherView* obj)
	{
		return obj->on_key_release_event(event);
	}
};

}

#endif // WHISKERMENU_LAUNCHER_VIEW_HPP
