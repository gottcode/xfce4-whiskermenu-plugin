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

#ifndef WHISKERMENU_PAGE_H
#define WHISKERMENU_PAGE_H

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class DesktopAction;
class Launcher;
class LauncherView;
class Window;

class Page
{
public:
	explicit Page(WhiskerMenu::Window *window);
	virtual ~Page();

	Page(const Page&) = delete;
	Page(Page&&) = delete;
	Page& operator=(const Page&) = delete;
	Page& operator=(Page&&) = delete;

	GtkWidget* get_widget() const
	{
		return m_widget;
	}

	LauncherView* get_view() const
	{
		return m_view;
	}

	void reset_selection();
	void update_view();

protected:
	Window* get_window() const
	{
		return m_window;
	}

	virtual void view_created()
	{
	}

	void set_reorderable(bool reorderable);

private:
	void create_view();
	virtual bool remember_launcher(Launcher* launcher);
	void launcher_activated(GtkTreePath* path);
	void launcher_action_activated(GtkMenuItem* menuitem, DesktopAction* action);
	gboolean view_button_press_event(GtkWidget* view, GdkEvent* event);
	gboolean view_button_release_event(GtkWidget*, GdkEvent* event);
	void view_drag_data_get(GtkWidget*, GdkDragContext*, GtkSelectionData* data, guint info, guint);
	void view_drag_end(GtkWidget*, GdkDragContext*);
	gboolean view_popup_menu_event(GtkWidget* view);
	void on_unmap();
	void destroy_context_menu(GtkMenuShell* menu);
	void add_selected_to_desktop();
	void add_selected_to_panel();
	void add_selected_to_favorites();
	void edit_selected();
	void hide_selected();
	void remove_selected_from_favorites();
	void create_context_menu(GtkTreePath* path, GdkEvent* event);
	virtual void extend_context_menu(GtkWidget* menu);

	static void item_activated_slot(GtkIconView*, GtkTreePath* path, gpointer user_data)
	{
		static_cast<Page*>(user_data)->launcher_activated(path);
	}

	static void row_activated_slot(GtkTreeView*, GtkTreePath* path, GtkTreeViewColumn*, gpointer user_data)
	{
		static_cast<Page*>(user_data)->launcher_activated(path);
	}

private:
	Window* m_window;
	GtkWidget* m_widget;
	LauncherView* m_view;
	Launcher* m_selected_launcher;
	bool m_drag_enabled;
	bool m_launcher_dragged;
	bool m_reorderable;
};

}

#endif // WHISKERMENU_PAGE_H
