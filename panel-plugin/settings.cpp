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

#include "settings.h"

#include "command.h"
#include "plugin.h"
#include "search-action.h"

#include <algorithm>

#include <libxfce4util/libxfce4util.h>

#include <unistd.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

Settings* WhiskerMenu::wm_settings = nullptr;

static const gchar* const settings_command[Settings::CountCommands][2] = {
	{ "command-settings",   "show-command-settings"   },
	{ "command-lockscreen", "show-command-lockscreen" },
	{ "command-switchuser", "show-command-switchuser" },
	{ "command-logoutuser", "show-command-logoutuser" },
	{ "command-restart",    "show-command-restart"    },
	{ "command-shutdown",   "show-command-shutdown"   },
	{ "command-suspend",    "show-command-suspend"    },
	{ "command-hibernate",  "show-command-hibernate"  },
	{ "command-logout",     "show-command-logout"     },
	{ "command-menueditor", "show-command-menueditor" },
	{ "command-profile",    "show-command-profile"    }
};

//-----------------------------------------------------------------------------

static void read_vector_entry(XfceRc* rc, const gchar* key, std::vector<std::string>& desktop_ids)
{
	if (!xfce_rc_has_entry(rc, key))
	{
		return;
	}

	desktop_ids.clear();

	gchar** values = xfce_rc_read_list_entry(rc, key, ",");
	for (size_t i = 0; values[i]; ++i)
	{
		std::string desktop_id(values[i]);
		if (std::find(desktop_ids.cbegin(), desktop_ids.cend(), desktop_id) == desktop_ids.cend())
		{
			desktop_ids.push_back(std::move(desktop_id));
		}
	}
	g_strfreev(values);
}

//-----------------------------------------------------------------------------

static void write_vector_entry(XfceRc* rc, const gchar* key, const std::vector<std::string>& desktop_ids)
{
	const auto size = desktop_ids.size();
	gchar** values = g_new0(gchar*, size + 1);
	for (decltype(desktop_ids.size()) i = 0; i < size; ++i)
	{
		values[i] = g_strdup(desktop_ids[i].c_str());
	}
	xfce_rc_write_list_entry(rc, key, values, ",");
	g_strfreev(values);
}

//-----------------------------------------------------------------------------

Settings::Settings() :
	m_button_title_default(_("Applications")),
	m_modified(false),

	favorites {
		"exo-web-browser.desktop",
		"exo-mail-reader.desktop",
		"exo-file-manager.desktop",
		"exo-terminal-emulator.desktop"
	},

	button_title(m_button_title_default),
	button_icon_name("xfce4-whiskermenu"),
	button_title_visible(false),
	button_icon_visible(true),
	button_single_row(false),

	launcher_show_name(true),
	launcher_show_description(true),
	launcher_show_tooltip(true),
	launcher_icon_size(IconSize::Normal),

	category_hover_activate(false),
	category_show_name(true),
	category_icon_size(IconSize::Smaller),

	load_hierarchy(false),
	view_as_icons(true),

	recent_items_max(10),
	favorites_in_recent(true),
	display_recent(false),

	position_search_alternate(true),
	position_commands_alternate(false),
	position_categories_alternate(true),
	stay_on_focus_out(false),

	confirm_session_command(true),

	search_actions {
		new SearchAction(_("Man Pages"), "#", "exo-open --launch TerminalEmulator man %s", false, true),
		new SearchAction(_("Web Search"), "?", "exo-open --launch WebBrowser https://duckduckgo.com/?q=%u", false, true),
		new SearchAction(_("Wikipedia"), "!w", "exo-open --launch WebBrowser https://en.wikipedia.org/wiki/%u", false, true),
		new SearchAction(_("Run in Terminal"), "!", "exo-open --launch TerminalEmulator %s", false, true),
		new SearchAction(_("Open URI"), "^(file|http|https):\\/\\/(.*)$", "exo-open \\0", true, true)
	},

	menu_width(450),
	menu_height(500),
	menu_opacity(100)
{
	command[CommandSettings] = new Command("org.xfce.settings.manager", "preferences-desktop",
			_("_Settings Manager"),
			"xfce4-settings-manager",
			_("Failed to open settings manager."));
	command[CommandLockScreen] = new Command("xfsm-lock", "system-lock-screen",
			_("_Lock Screen"),
			"xflock4",
			_("Failed to lock screen."));
	command[CommandSwitchUser] = new Command("xfsm-switch-user", "system-users",
			_("Switch _User"),
			"gdmflexiserver",
			_("Failed to switch user."));
	command[CommandLogOutUser] = new Command("xfsm-logout", "system-log-out",
			_("Log _Out"),
			"xfce4-session-logout --logout --fast",
			_("Failed to log out."),
			_("Are you sure you want to log out?"),
			_("Logging out in %d seconds."));
	command[CommandRestart] = new Command("xfsm-reboot", "system-reboot",
			_("_Restart"),
			"xfce4-session-logout --reboot --fast",
			_("Failed to restart."),
			_("Are you sure you want to restart?"),
			_("Restarting computer in %d seconds."));
	command[CommandShutDown] = new Command("xfsm-shutdown", "system-shutdown",
			_("Shut _Down"),
			"xfce4-session-logout --halt --fast",
			_("Failed to shut down."),
			_("Are you sure you want to shut down?"),
			_("Turning off computer in %d seconds."));
	command[CommandSuspend] = new Command("xfsm-suspend", "system-suspend",
			_("Suspe_nd"),
			"xfce4-session-logout --suspend",
			_("Failed to suspend."),
			_("Do you want to suspend to RAM?"),
			_("Suspending computer in %d seconds."));
	command[CommandHibernate] = new Command("xfsm-hibernate", "system-hibernate",
			_("_Hibernate"),
			"xfce4-session-logout --hibernate",
			_("Failed to hibernate."),
			_("Do you want to suspend to disk?"),
			_("Hibernating computer in %d seconds."));
	command[CommandLogOut] = new Command("xfsm-logout", "system-log-out",
			_("Log Ou_t..."),
			"xfce4-session-logout",
			_("Failed to log out."));
	command[CommandMenuEditor] = new Command("xfce4-menueditor", "menu-editor",
			_("_Edit Applications"),
			"menulibre",
			_("Failed to launch menu editor."));
	command[CommandProfile] = new Command("avatar-default", "preferences-desktop-user",
			_("Edit _Profile"),
			"mugshot",
			_("Failed to edit profile."));
}

//-----------------------------------------------------------------------------

Settings::~Settings()
{
	for (auto i : command)
	{
		delete i;
	}

	for (auto action : search_actions)
	{
		delete action;
	}
}

//-----------------------------------------------------------------------------

void Settings::load(gchar* file)
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
	xfce_rc_set_group(rc, nullptr);

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
	launcher_show_tooltip = xfce_rc_read_bool_entry(rc, "launcher-show-tooltip", launcher_show_tooltip);
	launcher_icon_size = xfce_rc_read_int_entry(rc, "item-icon-size", launcher_icon_size);

	category_hover_activate = xfce_rc_read_bool_entry(rc, "hover-switch-category", category_hover_activate);
	category_icon_size = xfce_rc_read_int_entry(rc, "category-icon-size", category_icon_size);
	category_show_name = xfce_rc_read_bool_entry(rc, "category-show-name", category_show_name) || (category_icon_size == -1);

	if (xfce_rc_has_entry(rc, "view-as-icons"))
	{
		load_hierarchy = xfce_rc_read_bool_entry(rc, "load-hierarchy", load_hierarchy);
		view_as_icons = xfce_rc_read_bool_entry(rc, "view-as-icons", view_as_icons) && !load_hierarchy;
	}
	else if (xfce_rc_has_entry(rc, "load-hierarchy"))
	{
		load_hierarchy = xfce_rc_read_bool_entry(rc, "load-hierarchy", load_hierarchy);
		view_as_icons = false;
	}

	recent_items_max = std::max(0, xfce_rc_read_int_entry(rc, "recent-items-max", recent_items_max));
	favorites_in_recent = xfce_rc_read_bool_entry(rc, "favorites-in-recent", favorites_in_recent);
	display_recent = xfce_rc_read_bool_entry(rc, "display-recent-default", display_recent) && recent_items_max;

	position_search_alternate = xfce_rc_read_bool_entry(rc, "position-search-alternate", position_search_alternate);
	position_commands_alternate = xfce_rc_read_bool_entry(rc, "position-commands-alternate", position_commands_alternate);
	position_categories_alternate = xfce_rc_read_bool_entry(rc, "position-categories-alternate", position_categories_alternate);
	stay_on_focus_out = xfce_rc_read_bool_entry(rc, "stay-on-focus-out", stay_on_focus_out);

	confirm_session_command = xfce_rc_read_bool_entry(rc, "confirm-session-command", confirm_session_command);

	menu_width = std::max(10, xfce_rc_read_int_entry(rc, "menu-width", menu_width));
	menu_height = std::max(10, xfce_rc_read_int_entry(rc, "menu-height", menu_height));
	menu_opacity = std::min(100, std::max(0, xfce_rc_read_int_entry(rc, "menu-opacity", menu_opacity)));

	for (int i = 0; i < CountCommands; ++i)
	{
		command[i]->set(xfce_rc_read_entry(rc, settings_command[i][0], command[i]->get()));
		command[i]->set_shown(xfce_rc_read_bool_entry(rc, settings_command[i][1], command[i]->get_shown()));
		command[i]->check();
	}

	int actions_count = xfce_rc_read_int_entry(rc, "search-actions", -1);
	if (actions_count > -1)
	{
		for (auto action : search_actions)
		{
			delete action;
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

void Settings::save(gchar* file)
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
	xfce_rc_set_group(rc, nullptr);

	write_vector_entry(rc, "favorites", favorites);
	write_vector_entry(rc, "recent", recent);

	if (!custom_menu_file.empty())
	{
		xfce_rc_write_entry(rc, "custom-menu-file", custom_menu_file.c_str());
	}

	if (button_title != Plugin::get_button_title_default())
	{
		xfce_rc_write_entry(rc, "button-title", button_title.c_str());
	}
	xfce_rc_write_entry(rc, "button-icon", button_icon_name.c_str());
	xfce_rc_write_bool_entry(rc, "button-single-row", button_single_row);
	xfce_rc_write_bool_entry(rc, "show-button-title", button_title_visible);
	xfce_rc_write_bool_entry(rc, "show-button-icon", button_icon_visible);

	xfce_rc_write_bool_entry(rc, "launcher-show-name", launcher_show_name);
	xfce_rc_write_bool_entry(rc, "launcher-show-description", launcher_show_description);
	xfce_rc_write_bool_entry(rc, "launcher-show-tooltip", launcher_show_tooltip);
	xfce_rc_write_int_entry(rc, "item-icon-size", launcher_icon_size);

	xfce_rc_write_bool_entry(rc, "hover-switch-category", category_hover_activate);
	xfce_rc_write_bool_entry(rc, "category-show-name", category_show_name);
	xfce_rc_write_int_entry(rc, "category-icon-size", category_icon_size);

	xfce_rc_write_bool_entry(rc, "load-hierarchy", load_hierarchy);
	xfce_rc_write_bool_entry(rc, "view-as-icons", view_as_icons);

	xfce_rc_write_int_entry(rc, "recent-items-max", recent_items_max);
	xfce_rc_write_bool_entry(rc, "favorites-in-recent", favorites_in_recent);
	xfce_rc_write_bool_entry(rc, "display-recent-default", display_recent);

	xfce_rc_write_bool_entry(rc, "position-search-alternate", position_search_alternate);
	xfce_rc_write_bool_entry(rc, "position-commands-alternate", position_commands_alternate);
	xfce_rc_write_bool_entry(rc, "position-categories-alternate", position_categories_alternate);
	xfce_rc_write_bool_entry(rc, "stay-on-focus-out", stay_on_focus_out);

	xfce_rc_write_bool_entry(rc, "confirm-session-command", confirm_session_command);

	xfce_rc_write_int_entry(rc, "menu-width", menu_width);
	xfce_rc_write_int_entry(rc, "menu-height", menu_height);
	xfce_rc_write_int_entry(rc, "menu-opacity", menu_opacity);

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
