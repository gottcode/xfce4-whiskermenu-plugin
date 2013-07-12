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


#ifndef WHISKERMENU_MENU_HPP
#define WHISKERMENU_MENU_HPP

#include "slot.hpp"

#include <map>
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
class ResizerWidget;
class RecentPage;
class SearchPage;
class SectionButton;

class Menu
{
public:
	explicit Menu(XfceRc* settings);
	~Menu();

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
	void save(XfceRc* settings);
	void set_categories(const std::vector<SectionButton*>& categories);
	void set_items(const std::map<std::string, Launcher*>& items);
	void set_modified();
	void unset_items();

private:
	SLOT_2(gboolean, Menu, on_enter_notify_event, GtkWidget*, GdkEventCrossing*);
	SLOT_2(gboolean, Menu, on_leave_notify_event, GtkWidget*, GdkEventCrossing*);
	SLOT_2(gboolean, Menu, on_button_press_event, GtkWidget*, GdkEventButton*);
	SLOT_2(gboolean, Menu, on_key_press_event, GtkWidget*, GdkEventKey*);
	SLOT_2(gboolean, Menu, on_key_press_event_after, GtkWidget*, GdkEventKey*);
	SLOT_2(gboolean, Menu, on_map_event, GtkWidget*, GdkEventAny*);
	SLOT_2(gboolean, Menu, on_configure_event, GtkWidget*, GdkEventConfigure*);
	SLOT_0(void, Menu, favorites_toggled);
	SLOT_0(void, Menu, recent_toggled);
	SLOT_0(void, Menu, category_toggled);
	SLOT_0(void, Menu, show_favorites);
	SLOT_0(void, Menu, search);
	SLOT_0(void, Menu, launch_settings_manager);
	SLOT_0(void, Menu, lock_screen);
	SLOT_0(void, Menu, log_out);

private:
	GtkWindow* m_window;

	GtkBox* m_vbox;
	GtkBox* m_title_box;
	GtkBox* m_contents_box;
	GtkBox* m_panels_box;
	GtkBox* m_sidebar_box;

	GtkLabel* m_username;
	GtkButton* m_settings_button;
	GtkButton* m_lock_screen_button;
	GtkButton* m_log_out_button;
	ResizerWidget* m_resizer;

	GtkEntry* m_search_entry;

	SearchPage* m_search_results;
	FavoritesPage* m_favorites;
	RecentPage* m_recent;
	ApplicationsPage* m_applications;

	GtkScrolledWindow* m_sidebar;
	SectionButton* m_favorites_button;
	SectionButton* m_recent_button;

	GdkRectangle m_geometry;
	bool m_layout_left;
	bool m_layout_bottom;
	bool m_modified;
};

}

#endif // WHISKERMENU_MENU_HPP
