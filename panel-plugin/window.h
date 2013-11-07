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

#ifndef WHISKERMENU_WINDOW_H
#define WHISKERMENU_WINDOW_H

#include <string>
#include <vector>

extern "C"
{
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
}

namespace WhiskerMenu
{

class ApplicationsPage;
class FavoritesPage;
class Launcher;
class Page;
class ResizerWidget;
class RecentPage;
class SearchPage;
class SectionButton;

class Window
{
public:
	explicit Window();
	~Window();

	GtkWidget* get_widget() const
	{
		return GTK_WIDGET(m_window);
	}

	GtkEntry* get_search_entry() const
	{
		return m_search_entry;
	}

	ApplicationsPage* get_applications() const
	{
		return m_applications;
	}

	FavoritesPage* get_favorites() const
	{
		return m_favorites;
	}

	RecentPage* get_recent() const
	{
		return m_recent;
	}

	bool get_modified() const
	{
		return m_modified;
	}

	void hide();
	void show(GtkWidget* parent, bool horizontal);
	void save();
	void set_categories(const std::vector<SectionButton*>& categories);
	void set_items();
	void set_loaded();
	void set_modified();
	void unset_items();

private:
	bool on_enter_notify_event(GdkEventCrossing* event);
	bool on_leave_notify_event(GdkEventCrossing* event);
	bool on_focus_in_event();
	bool on_button_press_event(GdkEventButton* event);
	bool on_key_press_event(GtkWidget* widget, GdkEventKey* event);
	bool on_key_press_event_after(GtkWidget* widget, GdkEventKey* event);
	bool on_map_event();
	bool on_configure_event(GdkEventConfigure* event);
	void favorites_toggled();
	void recent_toggled();
	void category_toggled();
	void show_favorites();
	void show_default_page();
	void search();

private:
	GtkWindow* m_window;

	GtkBox* m_window_box;
	GtkWidget* m_window_contents;
	GtkSpinner* m_window_load_spinner;
	GtkWidget* m_window_load_contents;

	GtkBox* m_vbox;
	GtkBox* m_title_box;
	GtkBox* m_commands_box;
	GtkBox* m_search_box;
	GtkBox* m_contents_box;
	GtkBox* m_panels_box;
	GtkBox* m_sidebar_box;

	GtkLabel* m_username;
	ResizerWidget* m_resizer;

	GtkAlignment* m_commands_align;

	GtkEntry* m_search_entry;

	SearchPage* m_search_results;
	FavoritesPage* m_favorites;
	RecentPage* m_recent;
	ApplicationsPage* m_applications;
	Page* m_default_page;

	GtkScrolledWindow* m_sidebar;
	SectionButton* m_favorites_button;
	SectionButton* m_recent_button;
	SectionButton* m_default_button;

	GdkRectangle m_geometry;
	bool m_layout_left;
	bool m_layout_bottom;
	bool m_layout_search_alternate;
	bool m_layout_commands_alternate;
	bool m_modified;


private:
	static gboolean on_enter_notify_event_slot(GtkWidget*, GdkEventCrossing* event, Window* obj)
	{
		return obj->on_enter_notify_event(event);
	}

	static gboolean on_leave_notify_event_slot(GtkWidget*, GdkEventCrossing* event, Window* obj)
	{
		return obj->on_leave_notify_event(event);
	}

	static gboolean on_focus_in_event_slot(GtkWidget*, GdkEventFocus*, Window* obj)
	{
		return obj->on_focus_in_event();
	}

	static gboolean on_button_press_event_slot(GtkWidget*, GdkEventButton* event, Window* obj)
	{
		return obj->on_button_press_event(event);
	}

	static gboolean on_key_press_event_slot(GtkWidget* widget, GdkEventKey* event, Window* obj)
	{
		return obj->on_key_press_event(widget, event);
	}

	static gboolean on_key_press_event_after_slot(GtkWidget* widget, GdkEventKey* event, Window* obj)
	{
		return obj->on_key_press_event_after(widget, event);
	}

	static gboolean on_map_event_slot(GtkWidget*, GdkEventAny*, Window* obj)
	{
		return obj->on_map_event();
	}

	static gboolean on_configure_event_slot(GtkWidget*, GdkEventConfigure* event, Window* obj)
	{
		return obj->on_configure_event(event);
	}

	static void favorites_toggled_slot(GtkToggleButton*, Window* obj)
	{
		obj->favorites_toggled();
	}

	static void recent_toggled_slot(GtkToggleButton*, Window* obj)
	{
		obj->recent_toggled();
	}

	static void category_toggled_slot(GtkToggleButton*, Window* obj)
	{
		obj->category_toggled();
	}

	static void show_favorites_slot(GtkTreeModel*, GtkTreePath*, GtkTreeIter*, Window* obj)
	{
		obj->show_favorites();
	}

	static void search_slot(GtkEditable*, Window* obj)
	{
		obj->search();
	}

	static void hide_slot(Window* obj)
	{
		obj->hide();
	}
};

}

#endif // WHISKERMENU_WINDOW_H
