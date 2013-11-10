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

#include "settings.h"

#include "command.h"

#include <algorithm>

extern "C"
{
#include <libxfce4util/libxfce4util.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

Settings* WhiskerMenu::wm_settings = NULL;

//-----------------------------------------------------------------------------

static void read_vector_entry(XfceRc* rc, const char* key, std::vector<std::string>& desktop_ids)
{
	if (!xfce_rc_has_entry(rc, key))
	{
		return;
	}

	desktop_ids.clear();

	gchar** values = xfce_rc_read_list_entry(rc, key, ",");
	for (size_t i = 0; values[i] != NULL; ++i)
	{
		std::string desktop_id(values[i]);
		if (std::find(desktop_ids.begin(), desktop_ids.end(), desktop_id) == desktop_ids.end())
		{
			desktop_ids.push_back(desktop_id);
		}
	}
	g_strfreev(values);
}

//-----------------------------------------------------------------------------

static void write_vector_entry(XfceRc* rc, const char* key, const std::vector<std::string>& desktop_ids)
{
	const std::vector<std::string>::size_type size = desktop_ids.size();
	gchar** values = g_new0(gchar*, size + 1);
	for (std::vector<std::string>::size_type i = 0; i < size; ++i)
	{
		values[i] = g_strdup(desktop_ids[i].c_str());
	}
	xfce_rc_write_list_entry(rc, key, values, ",");
	g_strfreev(values);
}

//-----------------------------------------------------------------------------

Settings::Settings() :
	button_icon_name("xfce4-whiskermenu"),
	button_title_visible(false),
	button_icon_visible(true),

	launcher_show_name(true),
	launcher_show_description(true),
	launcher_icon_size(IconSize::Small),

	category_hover_activate(false),
	category_icon_size(IconSize::Smaller),

	load_hierarchy(false),
	favorites_in_recent(true),

	display_recent(false),
	position_search_alternate(false),
	position_commands_alternate(false),

	menu_width(400),
	menu_height(500)
{
	favorites.push_back("exo-terminal-emulator.desktop");
	favorites.push_back("exo-file-manager.desktop");
	favorites.push_back("exo-mail-reader.desktop");
	favorites.push_back("exo-web-browser.desktop");

	command_settings = new Command("preferences-desktop", _("All _Settings"), "xfce4-settings-manager", _("Failed to open settings manager."));
	command_lockscreen = new Command("system-lock-screen", _("_Lock Screen"), "xflock4", _("Failed to lock screen."));
	command_logout = new Command("system-log-out", _("Log _Out"), "xfce4-session-logout", _("Failed to log out."));
	command_menueditor = new Command("xfce4-menueditor", _("_Edit Applications"), "menulibre", _("Failed to launch menu editor."));
}

//-----------------------------------------------------------------------------

Settings::~Settings()
{
	delete command_settings;
	delete command_lockscreen;
	delete command_logout;
	delete command_menueditor;
}

//-----------------------------------------------------------------------------

void Settings::load(char* file)
{
	if (!file)
	{
		return;
	}

	XfceRc* rc = xfce_rc_simple_open(file, true);
	g_free(file);
	if (!rc)
	{
		return;
	}

	read_vector_entry(rc, "favorites", favorites);
	read_vector_entry(rc, "recent", recent);

	button_title = xfce_rc_read_entry(rc, "button-title", button_title.c_str());
	button_icon_name = xfce_rc_read_entry(rc, "button-icon", button_icon_name.c_str());
	button_title_visible = xfce_rc_read_bool_entry(rc, "show-button-title", button_title_visible);
	button_icon_visible = xfce_rc_read_bool_entry(rc, "show-button-icon", button_icon_visible);

	launcher_show_name = xfce_rc_read_bool_entry(rc, "launcher-show-name", launcher_show_name);
	launcher_show_description = xfce_rc_read_bool_entry(rc, "launcher-show-description", launcher_show_description);
	launcher_icon_size = xfce_rc_read_int_entry(rc, "item-icon-size", launcher_icon_size);

	category_hover_activate = xfce_rc_read_bool_entry(rc, "hover-switch-category", category_hover_activate);
	category_icon_size = xfce_rc_read_int_entry(rc, "category-icon-size", category_icon_size);

	load_hierarchy = xfce_rc_read_bool_entry(rc, "load-hierarchy", load_hierarchy);
	favorites_in_recent = xfce_rc_read_bool_entry(rc, "favorites-in-recent", favorites_in_recent);

	display_recent = xfce_rc_read_bool_entry(rc, "display-recent-default", display_recent);
	position_search_alternate = xfce_rc_read_bool_entry(rc, "position-search-alternate", position_search_alternate);
	position_commands_alternate = xfce_rc_read_bool_entry(rc, "position-commands-alternate", position_commands_alternate) && position_search_alternate;

	command_settings->set(xfce_rc_read_entry(rc, "command-settings", command_settings->get()));
	command_lockscreen->set(xfce_rc_read_entry(rc, "command-lockscreen", command_lockscreen->get()));
	command_logout->set(xfce_rc_read_entry(rc, "command-logout", command_logout->get()));
	command_menueditor->set(xfce_rc_read_entry(rc, "command-menueditor", command_menueditor->get()));

	command_settings->set_shown(xfce_rc_read_bool_entry(rc, "show-command-settings", command_settings->get_shown()));
	command_lockscreen->set_shown(xfce_rc_read_bool_entry(rc, "show-command-lockscreen", command_lockscreen->get_shown()));
	command_logout->set_shown(xfce_rc_read_bool_entry(rc, "show-command-logout", command_logout->get_shown()));
	command_menueditor->set_shown(xfce_rc_read_bool_entry(rc, "show-command-menueditor", command_menueditor->get_shown()));

	menu_width = std::max(300, xfce_rc_read_int_entry(rc, "menu-width", menu_width));
	menu_height = std::max(400, xfce_rc_read_int_entry(rc, "menu-height", menu_height));

	xfce_rc_close(rc);

	command_settings->check();
	command_lockscreen->check();
	command_logout->check();
	command_menueditor->check();
}

//-----------------------------------------------------------------------------

void Settings::save(char* file)
{
	if (!file)
	{
		return;
	}

	XfceRc* rc = xfce_rc_simple_open(file, false);
	g_free(file);
	if (!rc)
	{
		return;
	}

	write_vector_entry(rc, "favorites", favorites);
	write_vector_entry(rc, "recent", recent);

	xfce_rc_write_entry(rc, "button-title", button_title.c_str());
	xfce_rc_write_entry(rc, "button-icon", button_icon_name.c_str());
	xfce_rc_write_bool_entry(rc, "show-button-title", button_title_visible);
	xfce_rc_write_bool_entry(rc, "show-button-icon", button_icon_visible);

	xfce_rc_write_bool_entry(rc, "launcher-show-name", launcher_show_name);
	xfce_rc_write_bool_entry(rc, "launcher-show-description", launcher_show_description);
	xfce_rc_write_int_entry(rc, "item-icon-size", launcher_icon_size);

	xfce_rc_write_bool_entry(rc, "hover-switch-category", category_hover_activate);
	xfce_rc_write_int_entry(rc, "category-icon-size", category_icon_size);

	xfce_rc_write_bool_entry(rc, "load-hierarchy", load_hierarchy);
	xfce_rc_write_bool_entry(rc, "favorites-in-recent", favorites_in_recent);

	xfce_rc_write_bool_entry(rc, "display-recent-default", display_recent);
	xfce_rc_write_bool_entry(rc, "position-search-alternate", position_search_alternate);
	xfce_rc_write_bool_entry(rc, "position-commands-alternate", position_commands_alternate);

	xfce_rc_write_entry(rc, "command-settings", command_settings->get());
	xfce_rc_write_entry(rc, "command-lockscreen", command_lockscreen->get());
	xfce_rc_write_entry(rc, "command-logout", command_logout->get());
	xfce_rc_write_entry(rc, "command-menueditor", command_menueditor->get());

	xfce_rc_write_bool_entry(rc, "show-command-settings", command_settings->get_shown());
	xfce_rc_write_bool_entry(rc, "show-command-lockscreen", command_lockscreen->get_shown());
	xfce_rc_write_bool_entry(rc, "show-command-logout", command_logout->get_shown());
	xfce_rc_write_bool_entry(rc, "show-command-menueditor", command_menueditor->get_shown());

	xfce_rc_write_int_entry(rc, "menu-width", menu_width);
	xfce_rc_write_int_entry(rc, "menu-height", menu_height);

	xfce_rc_close(rc);
}

//-----------------------------------------------------------------------------
