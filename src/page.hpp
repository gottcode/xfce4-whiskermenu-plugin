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


#ifndef WHISKERMENU_PAGE_HPP
#define WHISKERMENU_PAGE_HPP

#include "slot.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class FavoritesPage;
class Launcher;
class LauncherView;
class Menu;

class Page
{
public:
	Page(Menu* menu);
	virtual ~Page();

	GtkWidget* get_widget() const
	{
		return m_widget;
	}

	LauncherView* get_view() const
	{
		return m_view;
	}

protected:
	Menu* get_menu() const
	{
		return m_menu;
	}

private:
	SLOT_3(void, Page, launcher_activated, GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*);
	SLOT_2(gboolean, Page, view_button_press_event, GtkWidget*, GdkEventButton*);
	SLOT_1(gboolean, Page, view_popup_menu_event, GtkWidget*);
	SLOT_0(void, Page, on_unmap);
	SLOT_1(void, Page, destroy_context_menu, GtkMenuShell*);
	SLOT_0(void, Page, add_selected_to_desktop);
	SLOT_0(void, Page, add_selected_to_panel);
	SLOT_0(void, Page, add_selected_to_favorites);
	SLOT_0(void, Page, remove_selected_from_favorites);

private:
	Launcher* get_selected_launcher() const;
	void create_context_menu(GtkTreeIter* iter, GdkEventButton* event);
	static void position_context_menu(GtkMenu*, gint* x, gint* y, gboolean* push_in, Page* page);

private:
	Menu* m_menu;
	GtkWidget* m_widget;
	LauncherView* m_view;
	GtkTreePath* m_selected_path;
};

}

#endif // WHISKERMENU_PAGE_HPP
