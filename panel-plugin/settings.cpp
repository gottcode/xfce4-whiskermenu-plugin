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
#include "search-action.h"

#include <algorithm>

#include <libxfce4util/libxfce4util.h>

#include <unistd.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

Settings* WhiskerMenu::wm_settings = NULL;

static const char* const settings_command[Settings::CountCommands][2] = {
	{ "command-settings",   "show-command-settings"   },
	{ "command-lockscreen", "show-command-lockscreen" },
	{ "command-switchuser", "show-command-switchuser" },
	{ "command-logout",     "show-command-logout"     },
	{ "command-menueditor", "show-command-menueditor" }
};

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
	m_modified(false),

	button_icon_name("xfce4-whiskermenu"),
	button_title_visible(false),
	button_icon_visible(true),
        button_single_row(false),

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
	position_categories_alternate(false),

	menu_width(400),
	menu_height(500)
{
	favorites.push_back("exo-terminal-emulator.desktop");
	favorites.push_back("exo-file-manager.desktop");
	favorites.push_back("exo-mail-reader.desktop");
	favorites.push_back("exo-web-browser.desktop");

	command[CommandSettings] = new Command("preferences-desktop", _("All _Settings"), "xfce4-settings-manager", _("Failed to open settings manager."));
	command[CommandLockScreen] = new Command("system-lock-screen", _("_Lock Screen"), "xflock4", _("Failed to lock screen."));
	command[CommandSwitchUser] = new Command("system-users", _("Switch _Users"), "gdmflexiserver", _("Failed to switch users."));
	command[CommandLogOut] = new Command("system-log-out", _("Log _Out"), "xfce4-session-logout", _("Failed to log out."));
	command[CommandMenuEditor] = new Command("xfce4-menueditor", _("_Edit Applications"), "menulibre", _("Failed to launch menu editor."));

	search_actions.push_back(new SearchAction(_("Man Pages"), "#", "exo-open --launch TerminalEmulator man %s", false, true));
	search_actions.push_back(new SearchAction(_("Wikipedia"), "!w", "exo-open --launch WebBrowser https://en.wikipedia.org/wiki/%u", false, true));
	search_actions.push_back(new SearchAction(_("Run in Terminal"), "!", "exo-open --launch TerminalEmulator %s", false, true));
	search_actions.push_back(new SearchAction(_("Open URI"), "^(file|http|https):\\/\\/(.*)$", "exo-open \\0", true, true));
}

//-----------------------------------------------------------------------------

Settings::~Settings()
{
	for (int i = 0; i < CountCommands; ++i)
	{
		delete command[i];
	}

	for (std::vector<SearchAction*>::size_type i = 0, end = search_actions.size(); i < end; ++i)
	{
		delete search_actions[i];
	}
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
	xfce_rc_set_group(rc, NULL);

	read_vector_entry(rc, "favorites", favorites);
	read_vector_entry(rc, "recent", recent);

	custom_menu_file = xfce_rc_read_entry(rc, "custom-menu-file", custom_menu_file.c_str());

	button_title = xfce_rc_read_entry(rc, "button-title", button_title.c_str());
	button_icon_name = xfce_rc_read_entry(rc, "button-icon", button_icon_name.c_str());
	button_single_row = xfce_rc_read_bool_entry(rc, "button-single-row", button_single_row);
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
	position_categories_alternate = xfce_rc_read_bool_entry(rc, "position-categories-alternate", position_categories_alternate);

	menu_width = std::max(300, xfce_rc_read_int_entry(rc, "menu-width", menu_width));
	menu_height = std::max(400, xfce_rc_read_int_entry(rc, "menu-height", menu_height));

	for (int i = 0; i < CountCommands; ++i)
	{
		command[i]->set(xfce_rc_read_entry(rc, settings_command[i][0], command[i]->get()));
		command[i]->set_shown(xfce_rc_read_bool_entry(rc, settings_command[i][1], command[i]->get_shown()));
		command[i]->check();
	}

	int actions_count = xfce_rc_read_int_entry(rc, "search-actions", -1);
	if (actions_count > -1)
	{
		for (std::vector<SearchAction*>::size_type i = 0, end = search_actions.size(); i < end; ++i)
		{
			delete search_actions[i];
		}
		search_actions.clear();

		for (int i = 0; i < actions_count; ++i)
		{
			gchar* key = g_strdup_printf("action%i", i);
			if (!xfce_rc_has_group(rc, key))
			{
				g_free(key);
				continue;
			}
			xfce_rc_set_group(rc, key);
			g_free(key);

			search_actions.push_back(new SearchAction(
					xfce_rc_read_entry(rc, "name", ""),
					xfce_rc_read_entry(rc, "pattern", ""),
					xfce_rc_read_entry(rc, "command", ""),
					xfce_rc_read_bool_entry(rc, "regex", false),
					launcher_show_description));
		}
	}

	xfce_rc_close(rc);

	m_modified = false;
}

//-----------------------------------------------------------------------------

void Settings::save(char* file)
{
	if (!file)
	{
		return;
	}

	// Start with fresh settings
	unlink(file);

	XfceRc* rc = xfce_rc_simple_open(file, false);
	g_free(file);
	if (!rc)
	{
		return;
	}
	xfce_rc_set_group(rc, NULL);

	write_vector_entry(rc, "favorites", favorites);
	write_vector_entry(rc, "recent", recent);

	if (!custom_menu_file.empty())
	{
		xfce_rc_write_entry(rc, "custom-menu-file", custom_menu_file.c_str());
	}

	xfce_rc_write_entry(rc, "button-title", button_title.c_str());
	xfce_rc_write_entry(rc, "button-icon", button_icon_name.c_str());
	xfce_rc_write_bool_entry(rc, "button-single-row", button_single_row);
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
	xfce_rc_write_bool_entry(rc, "position-categories-alternate", position_categories_alternate);

	xfce_rc_write_int_entry(rc, "menu-width", menu_width);
	xfce_rc_write_int_entry(rc, "menu-height", menu_height);

	for (int i = 0; i < CountCommands; ++i)
	{
		xfce_rc_write_entry(rc, settings_command[i][0], command[i]->get());
		xfce_rc_write_bool_entry(rc, settings_command[i][1], command[i]->get_shown());
	}

	int actions_count = search_actions.size();
	xfce_rc_write_int_entry(rc, "search-actions", actions_count);
	for (int i = 0; i < actions_count; ++i)
	{
		gchar* key = g_strdup_printf("action%i", i);
		xfce_rc_set_group(rc, key);
		g_free(key);

		const SearchAction* action = search_actions[i];
		xfce_rc_write_entry(rc, "name", action->get_name());
		xfce_rc_write_entry(rc, "pattern", action->get_pattern());
		xfce_rc_write_entry(rc, "command", action->get_command());
		xfce_rc_write_bool_entry(rc, "regex", action->get_is_regex());
	}

	xfce_rc_close(rc);

	m_modified = false;
}

//-----------------------------------------------------------------------------
