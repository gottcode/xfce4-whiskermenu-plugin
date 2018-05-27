/*
 * Copyright (C) 2013, 2014, 2015, 2016, 2018 Graeme Gott <graeme@gottcode.org>
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

#include <vector>

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class ApplicationsPage;
class FavoritesPage;
class Page;
class Plugin;
class ProfilePicture;
class ResizerWidget;
class RecentPage;
class SearchPage;
class SectionButton;

class Window
{
public:
	explicit Window(Plugin* plugin);
	~Window();

	enum Position
	{
		PositionHorizontal = GTK_ORIENTATION_HORIZONTAL,
		PositionVertical = GTK_ORIENTATION_VERTICAL,
		PositionAtCursor
	};

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

	void hide();
	void show(const Position position);
	void save();
	void on_context_menu_destroyed();
	void set_categories(const std::vector<SectionButton*>& categories);
	void set_items();
	void set_loaded();
	void unset_items();

private:
	gboolean on_enter_notify_event(GtkWidget*, GdkEvent* event);
	gboolean on_leave_notify_event(GtkWidget*, GdkEvent* event);
	gboolean on_button_press_event(GtkWidget*, GdkEvent* event);
	gboolean on_key_press_event(GtkWidget* widget, GdkEvent* event);
	gboolean on_key_press_event_after(GtkWidget* widget, GdkEvent* event);
	gboolean on_map_event(GtkWidget*, GdkEvent*);
	gboolean on_state_flags_changed_event(GtkWidget*widget, GtkStateFlags);
	gboolean on_configure_event(GtkWidget*, GdkEvent* event);
	void on_screen_changed_event(GtkWidget* widget, GdkScreen* old_screen);
	gboolean on_draw_event(GtkWidget* widget, cairo_t* cr);
	void favorites_toggled();
	void recent_toggled();
	void category_toggled();
	void show_favorites();
	void show_default_page();
	void search();

private:
	Plugin* m_plugin;

	GtkWindow* m_window;

	GtkStyleContext* context;
	GtkCssProvider* provider;

	GtkStack* m_window_stack;
	GtkSpinner* m_window_load_spinner;

	GtkBox* m_vbox;
	GtkBox* m_title_box;
	GtkBox* m_commands_box;
	GtkBox* m_search_box;
	GtkStack* m_contents_stack;
	GtkBox* m_contents_box;
	GtkStack* m_panels_stack;

	GtkStackTransitionType m_search_cover;
	GtkStackTransitionType m_search_uncover;

	ProfilePicture* m_profilepic;
	GtkLabel* m_username;
	ResizerWidget* m_resizer;

	GtkWidget* m_commands_spacer;
	GtkWidget* m_commands_button[4];
	gulong m_command_slots[4];

	GtkEntry* m_search_entry;

	SearchPage* m_search_results;
	FavoritesPage* m_favorites;
	RecentPage* m_recent;
	ApplicationsPage* m_applications;
	Page* m_default_page;

	GtkScrolledWindow* m_sidebar;
	GtkBox* m_sidebar_buttons;
	SectionButton* m_favorites_button;
	SectionButton* m_recent_button;
	SectionButton* m_default_button;
	GtkSizeGroup* m_sidebar_size_group;

	GdkRectangle m_geometry;
	bool m_layout_left;
	bool m_layout_bottom;
	bool m_layout_search_alternate;
	bool m_layout_commands_alternate;
	bool m_supports_alpha;
};

}

#endif // WHISKERMENU_WINDOW_H
