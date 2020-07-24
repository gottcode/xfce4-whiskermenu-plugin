/*
 * Copyright (C) 2013-2021 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_SETTINGS_H
#define WHISKERMENU_SETTINGS_H

#include "icon-size.h"

#include <string>
#include <vector>

#include <glib.h>

namespace WhiskerMenu
{

class Command;
class Plugin;
class SearchAction;

class Settings
{
	Settings();
	~Settings();

	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;
	Settings& operator=(const Settings&) = delete;
	Settings& operator=(Settings&&) = delete;

	void load(gchar* file);
	void save(gchar* file);

	std::string m_button_title_default;
	bool m_modified;

public:
	bool get_modified() const
	{
		return m_modified;
	}

	void set_modified()
	{
		m_modified = true;
	}

public:
	std::vector<std::string> favorites;
	std::vector<std::string> recent;

	std::string custom_menu_file;

	std::string button_title;
	std::string button_icon_name;
	bool button_title_visible;
	bool button_icon_visible;
	bool button_single_row;

	bool launcher_show_name;
	bool launcher_show_description;
	bool launcher_show_tooltip;
	IconSize launcher_icon_size;

	bool category_hover_activate;
	bool category_show_name;
	bool sort_categories;
	IconSize category_icon_size;

	bool load_hierarchy;
	bool view_as_icons;

	int default_category;

	decltype(recent.size()) recent_items_max;
	bool favorites_in_recent;

	bool position_search_alternate;
	bool position_commands_alternate;
	bool position_categories_alternate;
	bool position_categories_horizontal;
	bool stay_on_focus_out;

	enum Commands
	{
		CommandSettings = 0,
		CommandLockScreen,
		CommandSwitchUser,
		CommandLogOutUser,
		CommandRestart,
		CommandShutDown,
		CommandSuspend,
		CommandHibernate,
		CommandLogOut,
		CommandMenuEditor,
		CommandProfile,
		CountCommands
	};
	Command* command[CountCommands];
	bool confirm_session_command;

	std::vector<SearchAction*> search_actions;

	int menu_width;
	int menu_height;
	int menu_opacity;

	friend class Plugin;
};

extern Settings* wm_settings;

}

#endif // WHISKERMENU_SETTINGS_H
