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

#include "settings.h"

#include "command.h"
#include "plugin.h"
#include "search-action.h"

#include <algorithm>

#include <exo/exo.h>

#include <unistd.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

Settings* WhiskerMenu::wm_settings = nullptr;

//-----------------------------------------------------------------------------

Settings::Settings(const gchar* base) :
	m_button_title_default(_("Applications")),
	m_modified(false),
	channel(nullptr),

	favorites("/favorites", {
#if EXO_CHECK_VERSION(4,15,0)
		"xfce4-web-browser.desktop",
		"xfce4-mail-reader.desktop",
		"xfce4-file-manager.desktop",
		"xfce4-terminal-emulator.desktop"
#else
		"exo-web-browser.desktop",
		"exo-mail-reader.desktop",
		"exo-file-manager.desktop",
		"exo-terminal-emulator.desktop"
#endif
	}),
	recent("/recent", { }),

	custom_menu_file("/custom-menu-file"),

	button_title("/button-title", m_button_title_default),
	button_icon_name("/button-icon", "org.xfce.panel.whiskermenu"),
	button_title_visible("/show-button-title", false),
	button_icon_visible("/show-button-icon", true),
	button_single_row("/button-single-row", false),

	launcher_show_name("/launcher-show-name", true),
	launcher_show_description("/launcher-show-description", true),
	launcher_show_tooltip("/launcher-show-tooltip", true),
	launcher_icon_size("/launcher-icon-size", IconSize::Small),

	category_hover_activate("/hover-switch-category", false),
	category_show_name("/category-show-name", true),
	sort_categories("/sort-categories", true),
	category_icon_size("/category-icon-size", IconSize::Smaller),

	view_mode("/view-mode", ViewAsList, ViewAsIcons, ViewAsTree),

	default_category("/default-category", CategoryFavorites, CategoryFavorites, CategoryAll),

	recent_items_max("/recent-items-max", 10, 0, 100),
	favorites_in_recent("/favorites-in-recent", true),

	position_search_alternate("/position-search-alternate", false),
	position_commands_alternate("/position-commands-alternate", false),
	position_categories_alternate("/position-categories-alternate", false),
	position_categories_horizontal("/position-categories-horizontal", false),
	stay_on_focus_out("/stay-on-focus-out", false),

	profile_shape("/profile-shape", ProfileRound, ProfileRound, ProfileHidden),

	confirm_session_command("/confirm-session-command", true),

	search_actions {
		new SearchAction(_("Man Pages"), "#", "exo-open --launch TerminalEmulator man %s", false),
		new SearchAction(_("Search the Web"), "?", "exo-open --launch WebBrowser https://duckduckgo.com/?q=%u", false),
		new SearchAction(_("Search for Files"), "-", "catfish --path=~ --start %s", false),
		new SearchAction(_("Wikipedia"), "!w", "exo-open --launch WebBrowser https://en.wikipedia.org/wiki/%u", false),
		new SearchAction(_("Run in Terminal"), "!", "exo-open --launch TerminalEmulator %s", false),
		new SearchAction(_("Open URI"), "^(file|http|https):\\/\\/(.*)$", "exo-open \\0", true)
	},

	menu_width("/menu-width", 450, 10, INT_MAX),
	menu_height("/menu-height", 500, 10, INT_MAX),
	menu_opacity("/menu-opacity", 100, 0, 100)
{
	command[CommandSettings] = new Command("/command-settings", "/show-command-settings",
			"org.xfce.settings.manager", "preferences-desktop",
			_("_Settings Manager"),
			"xfce4-settings-manager", true,
			_("Failed to open settings manager."));
	command[CommandLockScreen] = new Command("/command-lockscreen", "/show-command-lockscreen",
			"xfsm-lock", "system-lock-screen",
			_("_Lock Screen"),
			"xflock4", true,
			_("Failed to lock screen."));
	command[CommandSwitchUser] = new Command("/command-switchuser", "/show-command-switchuser",
			"xfsm-switch-user", "system-users",
			_("Switch _User"),
			"xfce4-session-logout --switch-user", false,
			_("Failed to switch user."));
	command[CommandLogOutUser] = new Command("/command-logoutuser", "/show-command-logoutuser",
			"xfsm-logout", "system-log-out",
			_("Log _Out"),
			"xfce4-session-logout --logout --fast", false,
			_("Failed to log out."),
			_("Are you sure you want to log out?"),
			_("Logging out in %d seconds."));
	command[CommandRestart] = new Command("/command-restart", "/show-command-restart",
			"xfsm-reboot", "system-reboot",
			_("_Restart"),
			"xfce4-session-logout --reboot --fast", false,
			_("Failed to restart."),
			_("Are you sure you want to restart?"),
			_("Restarting computer in %d seconds."));
	command[CommandShutDown] = new Command("/command-shutdown", "/show-command-shutdown",
			"xfsm-shutdown", "system-shutdown",
			_("Shut _Down"),
			"xfce4-session-logout --halt --fast", false,
			_("Failed to shut down."),
			_("Are you sure you want to shut down?"),
			_("Turning off computer in %d seconds."));
	command[CommandSuspend] = new Command("/command-suspend", "/show-command-suspend",
			"xfsm-suspend", "system-suspend",
			_("Suspe_nd"),
			"xfce4-session-logout --suspend", false,
			_("Failed to suspend."),
			_("Do you want to suspend to RAM?"),
			_("Suspending computer in %d seconds."));
	command[CommandHibernate] = new Command("/command-hibernate", "/show-command-hibernate",
			"xfsm-hibernate", "system-hibernate",
			_("_Hibernate"),
			"xfce4-session-logout --hibernate", false,
			_("Failed to hibernate."),
			_("Do you want to suspend to disk?"),
			_("Hibernating computer in %d seconds."));
	command[CommandLogOut] = new Command("/command-logout", "/show-command-logout",
			"xfsm-logout", "system-log-out",
			_("Log Ou_t..."),
			"xfce4-session-logout", true,
			_("Failed to log out."));
	command[CommandMenuEditor] = new Command("/command-menueditor", "/show-command-menueditor",
			"menu-editor", "xfce4-menueditor",
			_("_Edit Applications"),
			"menulibre", true,
			_("Failed to launch menu editor."));
	command[CommandProfile] = new Command("/command-profile", "/show-command-profile",
			"avatar-default", "preferences-desktop-user",
			_("Edit _Profile"),
			"mugshot", true,
			_("Failed to edit profile."));

	if (base && xfconf_init(nullptr))
	{
		channel = xfconf_channel_new_with_property_base(xfce_panel_get_channel_name(), base);
	}
}

//-----------------------------------------------------------------------------

Settings::~Settings()
{
	for (auto i : command)
	{
		delete i;
	}

	if (channel)
	{
		g_object_unref(channel);
		xfconf_shutdown();
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

	favorites.load(rc);
	recent.load(rc);

	custom_menu_file.load(rc);

	button_title.load(rc);
	button_icon_name.load(rc);
	button_single_row.load(rc);
	button_title_visible.load(rc);
	button_icon_visible.load(rc);

	launcher_show_name.load(rc);
	launcher_show_description.load(rc);
	launcher_show_tooltip.load(rc);
	if (xfce_rc_has_entry(rc, "item-icon-size"))
	{
		launcher_icon_size = xfce_rc_read_int_entry(rc, "item-icon-size", launcher_icon_size);
	}
	launcher_icon_size.load(rc);

	category_hover_activate.load(rc);
	category_show_name.load(rc);
	category_icon_size.load(rc);

	if (!xfce_rc_has_entry(rc, "view-mode"))
	{
		if (xfce_rc_read_bool_entry(rc, "load-hierarchy", view_mode == ViewAsTree))
		{
			view_mode = ViewAsTree;
			if (!xfce_rc_has_entry(rc, "sort-categories"))
			{
				sort_categories = false;
			}
		}
		else if (xfce_rc_read_bool_entry(rc, "view-as-icons", view_mode == ViewAsIcons))
		{
			view_mode = ViewAsIcons;
		}
	}
	view_mode.load(rc);
	sort_categories.load(rc);

	if (xfce_rc_has_entry(rc, "display-recent-default"))
	{
		default_category = xfce_rc_read_bool_entry(rc, "display-recent-default", default_category);
	}
	default_category.load(rc);

	recent_items_max.load(rc);
	favorites_in_recent.load(rc);

	position_search_alternate.load(rc);
	position_commands_alternate.load(rc);
	position_categories_alternate.load(rc);
	position_categories_horizontal.load(rc);
	stay_on_focus_out.load(rc);

	profile_shape.load(rc);

	confirm_session_command.load(rc);

	menu_width.load(rc);
	menu_height.load(rc);
	menu_opacity.load(rc);

	for (auto i : command)
	{
		i->load(rc);
	}

	search_actions.load(rc);

	xfce_rc_close(rc);

	prevent_invalid();

	m_modified = false;
}

//-----------------------------------------------------------------------------

void Settings::load()
{
	if (!channel)
	{
		return;
	}

	favorites.load();
	recent.load();

	custom_menu_file.load();

	button_title.load();
	button_icon_name.load();
	button_title_visible.load();
	button_icon_visible.load();
	button_single_row.load();

	launcher_show_name.load();
	launcher_show_description.load();
	launcher_show_tooltip.load();
	launcher_icon_size.load();

	category_hover_activate.load();
	category_show_name.load();
	sort_categories.load();
	category_icon_size.load();

	view_mode.load();

	default_category.load();

	recent_items_max.load();
	favorites_in_recent.load();

	position_search_alternate.load();
	position_commands_alternate.load();
	position_categories_alternate.load();
	position_categories_horizontal.load();
	stay_on_focus_out.load();

	profile_shape.load();

	confirm_session_command.load();

	menu_width.load();
	menu_height.load();
	menu_opacity.load();

	for (auto i : command)
	{
		i->load();
	}

	search_actions.load();

	prevent_invalid();

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

	favorites.save(rc);
	recent.save(rc);

	if (!custom_menu_file.empty())
	{
		custom_menu_file.save(rc);
	}

	if (button_title != Plugin::get_button_title_default())
	{
		button_title.save(rc);
	}
	button_icon_name.save(rc);
	button_single_row.save(rc);
	button_title_visible.save(rc);
	button_icon_visible.save(rc);

	launcher_show_name.save(rc);
	launcher_show_description.save(rc);
	launcher_show_tooltip.save(rc);
	launcher_icon_size.save(rc);

	category_hover_activate.save(rc);
	category_show_name.save(rc);
	category_icon_size.save(rc);
	sort_categories.save(rc);

	view_mode.save(rc);

	default_category.save(rc);

	recent_items_max.save(rc);
	favorites_in_recent.save(rc);

	position_search_alternate.save(rc);
	position_commands_alternate.save(rc);
	position_categories_alternate.save(rc);
	position_categories_horizontal.save(rc);
	stay_on_focus_out.save(rc);

	profile_shape.save(rc);

	confirm_session_command.save(rc);

	menu_width.save(rc);
	menu_height.save(rc);
	menu_opacity.save(rc);

	for (auto i : command)
	{
		i->save(rc);
	}

	search_actions.save(rc);

	xfce_rc_close(rc);

	m_modified = false;
}

//-----------------------------------------------------------------------------

void Settings::prevent_invalid()
{
	// Prevent empty categories
	if (!category_show_name && (category_icon_size == -1))
	{
		category_show_name = true;
	}

	// Reset default category if recent is hidden
	if (!recent_items_max && (default_category == CategoryRecent))
	{
		default_category = CategoryFavorites;
	}

	// Prevent empty panel button
	if (!button_icon_visible)
	{
		if (!button_title_visible)
		{
			button_icon_visible = true;
		}
		else if (button_title.empty())
		{
			button_title = m_button_title_default;
		}
	}
}

//-----------------------------------------------------------------------------

Boolean::Boolean(const gchar* property, bool data) :
	m_property(property),
	m_data(data)
{
}

//-----------------------------------------------------------------------------

void Boolean::load(XfceRc* rc)
{
	set(xfce_rc_read_bool_entry(rc, m_property + 1, m_data));
}

//-----------------------------------------------------------------------------

void Boolean::load()
{
	set(xfconf_channel_get_bool(wm_settings->channel, m_property, m_data));
}

//-----------------------------------------------------------------------------

void Boolean::save(XfceRc* rc)
{
	xfce_rc_write_bool_entry(rc, m_property + 1, m_data);
}

//-----------------------------------------------------------------------------

void Boolean::set(bool data)
{
	if (m_data == data)
	{
		return;
	}

	m_data = data;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

Integer::Integer(const gchar* property, int data, int min, int max) :
	m_property(property),
	m_min(min),
	m_max(max),
	m_data(CLAMP(data, min, max))
{
}

//-----------------------------------------------------------------------------

void Integer::load(XfceRc* rc)
{
	set(xfce_rc_read_int_entry(rc, m_property + 1, m_data));
}

//-----------------------------------------------------------------------------

void Integer::load()
{
	set(xfconf_channel_get_int(wm_settings->channel, m_property, m_data));
}

//-----------------------------------------------------------------------------

void Integer::save(XfceRc* rc)
{
	xfce_rc_write_int_entry(rc, m_property + 1, m_data);
}

//-----------------------------------------------------------------------------

void Integer::set(int data)
{
	data = CLAMP(data, m_min, m_max);
	if (m_data == data)
	{
		return;
	}

	m_data = data;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

String::String(const gchar* property, const std::string& data) :
	m_property(property),
	m_data(data)
{
}

//-----------------------------------------------------------------------------

void String::load(XfceRc* rc)
{
	set(xfce_rc_read_entry(rc, m_property + 1, m_data.c_str()));
}

//-----------------------------------------------------------------------------

void String::load()
{
	gchar* value = xfconf_channel_get_string(wm_settings->channel, m_property, m_data.c_str());
	set(value);
	g_free(value);
}

//-----------------------------------------------------------------------------

void String::save(XfceRc* rc)
{
	xfce_rc_write_entry(rc, m_property + 1, m_data.c_str());
}

//-----------------------------------------------------------------------------

void String::set(const std::string& data)
{
	if (m_data == data)
	{
		return;
	}

	m_data = data;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

StringList::StringList(const gchar* property, std::initializer_list<std::string> data) :
	m_property(property),
	m_data(data)
{
}

//-----------------------------------------------------------------------------

void StringList::clear()
{
	m_data.clear();
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void StringList::erase(int pos)
{
	m_data.erase(m_data.begin() + pos);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void StringList::insert(int pos, const std::string& value)
{
	m_data.insert(m_data.begin() + pos, value);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void StringList::push_back(const std::string& value)
{
	m_data.push_back(value);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void StringList::resize(int count)
{
	m_data.resize(count);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void StringList::set(int pos, const std::string& value)
{
	m_data[pos] = value;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void StringList::load(XfceRc* rc)
{
	if (!xfce_rc_has_entry(rc, m_property + 1))
	{
		return;
	}

	gchar** data = xfce_rc_read_list_entry(rc, m_property + 1, ",");
	if (!data)
	{
		return;
	}

	std::vector<std::string> strings;
	for (int i = 0; data[i]; ++i)
	{
		strings.push_back(data[i]);
	}
	set(strings);

	g_strfreev(data);
}

//-----------------------------------------------------------------------------

void StringList::load()
{
	GValue value = G_VALUE_INIT;
	if (!xfconf_channel_get_property(wm_settings->channel, m_property, &value))
	{
		return;
	}

	// Convert GValue to string list
	std::vector<std::string> strings;
	if (G_VALUE_HOLDS(&value, G_TYPE_PTR_ARRAY))
	{
		const GPtrArray* values = static_cast<GPtrArray*>(g_value_get_boxed(&value));
		for (guint i = 0; i < values->len; ++i)
		{
			const GValue* string = static_cast<GValue*>(g_ptr_array_index(values, i));
			if (G_VALUE_HOLDS_STRING(string))
			{
				strings.push_back(g_value_get_string(string));
			}
		}
	}
	else if (G_VALUE_HOLDS(&value, G_TYPE_STRV))
	{
		const gchar** values = static_cast<const gchar**>(g_value_get_boxed(&value));
		for (int i = 0; values[i]; ++i)
		{
			strings.push_back(values[i]);
		}
	}
	else if (G_VALUE_HOLDS_STRING(&value))
	{
		strings.push_back(g_value_get_string(&value));
	}
	else
	{
		return;
	}

	// Load string list
	set(strings);

	g_value_unset(&value);

	return;
}

//-----------------------------------------------------------------------------

void StringList::save(XfceRc* rc)
{
	const int size = m_data.size();
	gchar** values = g_new0(gchar*, size + 1);
	for (int i = 0; i < size; ++i)
	{
		values[i] = g_strdup(m_data[i].c_str());
	}
	xfce_rc_write_list_entry(rc, m_property + 1, values, ",");
	g_strfreev(values);
}

//-----------------------------------------------------------------------------

void StringList::set(std::vector<std::string>& data)
{
	m_data.clear();

	for (auto& desktop_id : data)
	{
#if EXO_CHECK_VERSION(4,15,0)
		if (desktop_id == "exo-web-browser.desktop")
		{
			desktop_id = "xfce4-web-browser.desktop";
		}
		else if (desktop_id == "exo-mail-reader.desktop")
		{
			desktop_id = "xfce4-mail-reader.desktop";
		}
		else if (desktop_id == "exo-file-manager.desktop")
		{
			desktop_id = "xfce4-file-manager.desktop";
		}
		else if (desktop_id == "exo-terminal-emulator.desktop")
		{
			desktop_id = "xfce4-terminal-emulator.desktop";
		}
#endif
		if (std::find(m_data.begin(), m_data.end(), desktop_id) == m_data.end())
		{
			m_data.push_back(std::move(desktop_id));
		}
	}
}

//-----------------------------------------------------------------------------

SearchActionList::SearchActionList(std::initializer_list<SearchAction*> data) :
	m_data(data)
{
}

//-----------------------------------------------------------------------------

SearchActionList::~SearchActionList()
{
	for (auto action : m_data)
	{
		delete action;
	}
}

//-----------------------------------------------------------------------------

void SearchActionList::erase(SearchAction* value)
{
	m_data.erase(std::find(m_data.begin(), m_data.end(), value));
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SearchActionList::push_back(SearchAction* value)
{
	m_data.push_back(value);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SearchActionList::load(XfceRc* rc)
{
	const int size = xfce_rc_read_int_entry(rc, "search-actions", -1);
	if (size < 0)
	{
		return;
	}

	for (auto action : m_data)
	{
		delete action;
	}
	m_data.clear();

	for (int i = 0; i < size; ++i)
	{
		gchar* key = g_strdup_printf("action%i", i);
		if (!xfce_rc_has_group(rc, key))
		{
			g_free(key);
			continue;
		}
		xfce_rc_set_group(rc, key);
		g_free(key);

		m_data.push_back(new SearchAction(
				xfce_rc_read_entry(rc, "name", ""),
				xfce_rc_read_entry(rc, "pattern", ""),
				xfce_rc_read_entry(rc, "command", ""),
				xfce_rc_read_bool_entry(rc, "regex", false)));
	}

	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SearchActionList::load()
{
	const int size = xfconf_channel_get_int(wm_settings->channel, "/search-actions", -1);
	if (size < 0)
	{
		return;
	}

	for (auto action : m_data)
	{
		delete action;
	}
	m_data.clear();

	gchar* property = nullptr;
	gchar* name = nullptr;
	gchar* pattern = nullptr;
	gchar* command = nullptr;
	bool regex;

	for (int i = 0; i < size; ++i)
	{
		property = g_strdup_printf("/search-actions/action-%d/name", i);
		name = xfconf_channel_get_string(wm_settings->channel, property, nullptr);
		g_free(property);

		property = g_strdup_printf("/search-actions/action-%d/pattern", i);
		pattern = xfconf_channel_get_string(wm_settings->channel, property, nullptr);
		g_free(property);

		property = g_strdup_printf("/search-actions/action-%d/command", i);
		command = xfconf_channel_get_string(wm_settings->channel, property, nullptr);
		g_free(property);

		property = g_strdup_printf("/search-actions/action-%d/regex", i);
		regex = xfconf_channel_get_bool(wm_settings->channel, property, false);
		g_free(property);

		m_data.push_back(new SearchAction(name, pattern, command, regex));

		g_free(name);
		g_free(pattern);
		g_free(command);
	}
}

//-----------------------------------------------------------------------------

void SearchActionList::save(XfceRc* rc)
{
	const int size = m_data.size();
	xfce_rc_write_int_entry(rc, "search-actions", size);

	for (int i = 0; i < size; ++i)
	{
		gchar* key = g_strdup_printf("action%i", i);
		xfce_rc_set_group(rc, key);
		g_free(key);

		const SearchAction* action = m_data[i];
		xfce_rc_write_entry(rc, "name", action->get_name());
		xfce_rc_write_entry(rc, "pattern", action->get_pattern());
		xfce_rc_write_entry(rc, "command", action->get_command());
		xfce_rc_write_bool_entry(rc, "regex", action->get_is_regex());
	}
}

//-----------------------------------------------------------------------------
