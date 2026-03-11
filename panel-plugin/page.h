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

#ifndef WHISKERMENU_PAGE_H
#define WHISKERMENU_PAGE_H

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class CategoryButton;
class DesktopAction;
class Launcher;
class LauncherView;
class Settings;
class Window;

class Page
{
public:
	Page(Settings* settings, Window* window, const gchar* icon, const gchar* text);
	virtual ~Page();

	Page(const Page&) = delete;
	Page(Page&&) = delete;
	Page& operator=(const Page&) = delete;
	Page& operator=(Page&&) = delete;

	GtkWidget* get_widget() const
	{
		return m_widget;
	}

	CategoryButton* get_button() const
	{
		return m_button;
	}

	LauncherView* get_view() const
	{
		return m_view;
	}

	void reset_selection();
	void select_first();
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

protected:
	Settings* const m_settings;

private:
	void create_view();
	virtual bool remember_launcher(Launcher* launcher);
	void launcher_activated(GtkTreePath* path);
	void launcher_action_activated(GtkMenuItem* menuitem, DesktopAction* action);
	gboolean view_button_press_event(GdkEvent* event);
	gboolean view_button_release_event(GdkEvent* event);
	void view_drag_data_get(GtkSelectionData* data, guint info);
	void view_drag_end();
	gboolean view_popup_menu_event();
	void add_selected_to_desktop();
	void add_selected_to_panel();
	void edit_selected();
	void create_context_menu(GtkTreePath* path, GdkEvent* event);
	virtual void extend_context_menu(GtkWidget* menu);

private:
	Window* m_window;
	CategoryButton* m_button;
	GtkWidget* m_widget;
	LauncherView* m_view;
	bool m_drag_enabled;
	bool m_launcher_dragged;
	bool m_reorderable;

protected:
	Launcher* m_selected_launcher;
};

}

#endif // WHISKERMENU_PAGE_H
