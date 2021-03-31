/*
 * Copyright (C) 2013-2020 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_LAUNCHER_TREE_VIEW_H
#define WHISKERMENU_LAUNCHER_TREE_VIEW_H

#include "launcher-view.h"

namespace WhiskerMenu
{

class LauncherTreeView : public LauncherView
{
public:
	LauncherTreeView();
	~LauncherTreeView();

	GtkWidget* get_widget() const
	{
		return GTK_WIDGET(m_view);
	}

	GtkTreePath* get_cursor() const;
	GtkTreePath* get_path_at_pos(int x, int y) const;
	GtkTreePath* get_selected_path() const;
	void activate_path(GtkTreePath* path);
	void scroll_to_path(GtkTreePath* path);
	void select_path(GtkTreePath* path);
	void set_cursor(GtkTreePath* path);

	void set_fixed_height_mode(bool fixed_height);
	void set_selection_mode(GtkSelectionMode mode);

	void hide_tooltips();
	void show_tooltips();

	void clear_selection();
	void collapse_all();

	void set_model(GtkTreeModel* model);
	void unset_model();

	void set_drag_source(GdkModifierType start_button_mask, const GtkTargetEntry* targets, gint n_targets, GdkDragAction actions);
	void set_drag_dest(const GtkTargetEntry* targets, gint n_targets, GdkDragAction actions);
	void unset_drag_source();
	void unset_drag_dest();

	void reload_icon_size();

private:
	void create_column();
	gboolean on_key_press_event(GtkWidget*, GdkEvent* event);
	gboolean on_key_release_event(GtkWidget*, GdkEvent* event);
	void on_row_activated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column);

private:
	GtkTreeView* m_view;
	GtkTreeViewColumn* m_column;
	int m_icon_size;
};

}

#endif // WHISKERMENU_LAUNCHER_TREE_VIEW_H
