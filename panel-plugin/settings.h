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

#ifndef WHISKERMENU_SETTINGS_H
#define WHISKERMENU_SETTINGS_H

#include "icon-size.h"

#include <string>
#include <vector>

namespace WhiskerMenu
{

class Command;
class Plugin;

class Settings
{
	Settings();
	Settings(const Settings&);
	Settings& operator=(const Settings&);
	~Settings();

	void load(char* file);
	void save(char* file);

public:
	std::vector<std::string> favorites;
	std::vector<std::string> recent;

	std::string button_title;
	std::string button_icon_name;
	bool button_title_visible;
	bool button_icon_visible;

	bool launcher_show_name;
	bool launcher_show_description;
	IconSize launcher_icon_size;

	bool category_hover_activate;
	IconSize category_icon_size;

	bool load_hierarchy;
	bool favorites_in_recent;

	bool display_recent;
	bool position_search_alternate;
	bool position_commands_alternate;

	Command* command_settings;
	Command* command_lockscreen;
	Command* command_logout;
	Command* command_menueditor;

	int menu_width;
	int menu_height;

	friend class Plugin;
};

extern Settings* wm_settings;

}

#endif // WHISKERMENU_SETTINGS_H
