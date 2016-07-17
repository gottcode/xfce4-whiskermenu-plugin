/*
 * Copyright (C) 2013, 2016 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_LAUNCHER_VIEW_H
#define WHISKERMENU_LAUNCHER_VIEW_H

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class IconSize;
class Launcher;
class Window;

class LauncherView
{
public:
	LauncherView(Window* window);
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

	void set_fixed_height_mode(bool fixed_height);
	void set_reorderable(bool reorderable);
	void set_selection_mode(GtkSelectionMode mode);

	void hide_tooltips();
	void show_tooltips();

	void collapse_all();

	GtkTreeModel* get_model() const
	{
		return m_model;
	}

	void set_model(GtkTreeModel* model);
	void unset_model();

	void reload_icon_size();

	enum Columns
	{
		COLUMN_ICON = 0,
		COLUMN_TEXT,
		COLUMN_TOOLTIP,
		COLUMN_LAUNCHER,
		N_COLUMNS
	};

private:
	void create_column();
	gboolean on_key_press_event(GtkWidget*, GdkEvent* event);
	gboolean on_key_release_event(GtkWidget*, GdkEvent* event);
	gboolean on_button_press_event(GtkWidget*, GdkEvent* event);
	gboolean on_button_release_event(GtkWidget*, GdkEvent* event);
	void on_drag_data_get(GtkWidget*, GdkDragContext*, GtkSelectionData* data, guint info, guint);
	void on_drag_end(GtkWidget*, GdkDragContext*);
	void on_row_activated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column);
	gboolean test_row_toggle();

private:
	Window* m_window;

	GtkTreeModel* m_model;
	GtkTreeView* m_view;
	GtkTreeViewColumn* m_column;
	int m_icon_size;

	Launcher* m_pressed_launcher;
	bool m_drag_enabled;
	bool m_launcher_dragged;
	bool m_row_activated;
	bool m_reorderable;
};

}

#endif // WHISKERMENU_LAUNCHER_VIEW_H
