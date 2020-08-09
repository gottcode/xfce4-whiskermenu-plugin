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

#include "launcher.h"

#include "query.h"
#include "settings.h"
#include "util.h"

#include <libxfce4ui/libxfce4ui.h>

using namespace WhiskerMenu;

constexpr GSpawnFlags g_spawnFlags = static_cast<GSpawnFlags>(G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL);

//-----------------------------------------------------------------------------

static std::string normalize(const gchar* string)
{
	std::string result;

	gchar* normalized = g_utf8_normalize(string, -1, G_NORMALIZE_DEFAULT);
	if (G_UNLIKELY(!normalized))
	{
		return result;
	}

	gchar* utf8 = g_utf8_casefold(normalized, -1);
	if (G_UNLIKELY(!utf8))
	{
		g_free(normalized);
		return result;
	}

	result = utf8;

	g_free(utf8);
	g_free(normalized);

	return result;
}

//-----------------------------------------------------------------------------

#if !LIBXFCE4UTIL_CHECK_VERSION(4,15,2)
static void replace_with_quoted_string(std::string& command, std::string::size_type& index, const gchar* unquoted)
{
	if (!xfce_str_is_empty(unquoted))
	{
		gchar* quoted = g_shell_quote(unquoted);
		command.replace(index, 2, quoted);
		index += strlen(quoted);
		g_free(quoted);
	}
	else
	{
		command.erase(index, 2);
	}
}

//-----------------------------------------------------------------------------

static void replace_with_quoted_string(std::string& command, std::string::size_type& index, const gchar* prefix, const gchar* unquoted)
{
	if (!xfce_str_is_empty(unquoted))
	{
		command.replace(index, 2, prefix);
		index += strlen(prefix);

		gchar* quoted = g_shell_quote(unquoted);
		command.insert(index, quoted);
		index += strlen(quoted);
		g_free(quoted);
	}
	else
	{
		command.erase(index, 2);
	}
}

//-----------------------------------------------------------------------------

static gchar* xfce_expand_desktop_entry_field_codes(const gchar* command, GSList* /*uri_list*/,
		const gchar* icon, const gchar* name, const gchar* uri, gboolean requires_terminal)
{
	std::string expanded(command);

	if (requires_terminal)
	{
		expanded.insert(0, "exo-open --launch TerminalEmulator ");
	}

	std::string::size_type length = expanded.length() - 1;
	for (std::string::size_type i = 0; i < length; ++i)
	{
		if (G_UNLIKELY(command[i] == '%'))
		{
			switch (command[i + 1])
			{
			case 'i':
				replace_with_quoted_string(expanded, i, "--icon ", icon);
				break;

			case 'c':
				replace_with_quoted_string(expanded, i, name);
				break;

			case 'k':
				replace_with_quoted_string(expanded, i, uri);
				break;

			case '%':
				expanded.erase(i, 1);
				break;

			case 'f':
				// unsupported, pass in a single file dropped on launcher
			case 'F':
				// unsupported, pass in a list of files dropped on launcher
			case 'u':
				// unsupported, pass in a single URL dropped on launcher
			case 'U':
				// unsupported, pass in a list of URLs dropped on launcher
			default:
				expanded.erase(i, 2);
				break;
			}
			length = expanded.length() - 1;
		}
	}
	return g_strdup(expanded.c_str());
}
#endif

//-----------------------------------------------------------------------------

Launcher::Launcher(GarconMenuItem* item) :
	m_item(item),
	m_search_flags(0)
{
	// Fetch icon
	const gchar* icon = garcon_menu_item_get_icon_name(m_item);
	set_icon(icon ? icon : "application-x-executable");

	// Fetch text
	const gchar* name = garcon_menu_item_get_name(m_item);
	if (G_UNLIKELY(!name) || !g_utf8_validate(name, -1, nullptr))
	{
		name = "";
	}

	const gchar* generic_name = garcon_menu_item_get_generic_name(m_item);
	if (G_UNLIKELY(!generic_name) || !g_utf8_validate(generic_name, -1, nullptr))
	{
		generic_name = "";
	}

	if (!wm_settings->launcher_show_name && !xfce_str_is_empty(generic_name))
	{
		std::swap(name, generic_name);
	}
	m_display_name = name;

	const gchar* details = garcon_menu_item_get_comment(m_item);
	if (!details || !g_utf8_validate(details, -1, nullptr))
	{
		details = generic_name;
	}

	// Create display text
	const gchar* direction = (gtk_widget_get_default_direction() != GTK_TEXT_DIR_RTL) ? "\342\200\216" : "\342\200\217";
	if (wm_settings->launcher_show_description && !wm_settings->view_as_icons)
	{
		set_text(g_markup_printf_escaped("%s<b>%s</b>\n%s%s", direction, m_display_name, direction, details));
	}
	else
	{
		set_text(g_markup_printf_escaped("%s%s", direction, m_display_name));
	}
	set_tooltip(details);

	// Create search text for display name
	m_search_name = normalize(m_display_name);
	m_search_generic_name = normalize(generic_name);
	m_search_comment = normalize(details);

	// Create search text for keywords
#if GARCON_CHECK_VERSION(0,6,2)
	GList* keywords = garcon_menu_item_get_keywords(m_item);
	for (GList* i = keywords; i; i = i->next)
	{
		const gchar* keyword = static_cast<gchar*>(i->data);
		if (!xfce_str_is_empty(keyword) && g_utf8_validate(keyword, -1, nullptr))
		{
			m_search_keywords.push_back(normalize(keyword));
		}
	}
#endif

	// Create search text for command
	const gchar* command = garcon_menu_item_get_command(m_item);
	if (!xfce_str_is_empty(command) && g_utf8_validate(command, -1, nullptr))
	{
		m_search_command = normalize(command);
	}

	// Fetch desktop actions
#ifdef GARCON_TYPE_MENU_ITEM_ACTION
	GList* actions = garcon_menu_item_get_actions(m_item);
	for (GList* i = actions; i; i = i->next)
	{
		GarconMenuItemAction* action = garcon_menu_item_get_action(m_item, static_cast<gchar*>(i->data));
		if (action)
		{
			m_actions.push_back(new DesktopAction(action));
		}
	}
	g_list_free(actions);
#endif
}

//-----------------------------------------------------------------------------

Launcher::~Launcher()
{
	for (auto action : m_actions)
	{
		delete action;
	}
}

//-----------------------------------------------------------------------------

// Adapted from https://git.xfce.org/xfce/xfce4-appfinder/tree/src/appfinder-window.c#n945
void Launcher::hide()
{
	// Look up the correct relative path
	const gchar* relpath = nullptr;
	gchar* uri = get_uri();
	if (uri)
	{
		gchar** dirs = xfce_resource_lookup_all(XFCE_RESOURCE_DATA, "applications/");
		for (size_t i = 0; dirs[i]; ++i)
		{
			if (g_str_has_prefix(uri + 7, dirs[i]))
			{
				relpath = uri + 7 + strlen(dirs[i]) - 13;
				break;
			}
		}
		g_strfreev(dirs);
	}
	if (!relpath)
	{
		g_free(uri);
		return;
	}

	gchar* path = xfce_resource_save_location(XFCE_RESOURCE_DATA, relpath, false);
	// I18N: the first %s will be replaced with desktop file path, the second with Hidden=true
	gchar* message = g_strdup_printf(_("To unhide it you have to manually "
			"remove the file \"%s\" or open the file and "
			"remove the line \"%s\"."), path, "Hidden=true");
	g_free(path);

	if (xfce_dialog_confirm(nullptr, nullptr, _("Hide Application"), message,
		_("Are you sure you want to hide \"%s\"?"), m_display_name))
	{
		XfceRc* rc = xfce_rc_config_open(XFCE_RESOURCE_DATA, relpath, false);
		xfce_rc_set_group(rc, G_KEY_FILE_DESKTOP_GROUP);
		xfce_rc_write_bool_entry(rc, G_KEY_FILE_DESKTOP_KEY_HIDDEN, true);
		xfce_rc_close(rc);
	}

	g_free(message);
	g_free(uri);
}

//-----------------------------------------------------------------------------

void Launcher::run(GdkScreen* screen) const
{
	// Expand the field codes
	const gchar* string = garcon_menu_item_get_command(m_item);
	if (xfce_str_is_empty(string))
	{
		return;
	}

	gchar* uri = garcon_menu_item_get_uri(m_item);
	gchar* command = xfce_expand_desktop_entry_field_codes(string,
			nullptr,
			garcon_menu_item_get_icon_name(m_item),
			garcon_menu_item_get_name(m_item),
			uri,
			garcon_menu_item_requires_terminal(m_item));
	g_free(uri);

	// Parse and spawn command
	gchar** argv;
	bool result = false;
	GError* error = nullptr;
	if (g_shell_parse_argv(command, nullptr, &argv, &error))
	{
		result = xfce_spawn_on_screen(screen,
				garcon_menu_item_get_path(m_item),
				argv, nullptr, g_spawnFlags,
				garcon_menu_item_supports_startup_notification(m_item),
				gtk_get_current_event_time(),
				garcon_menu_item_get_icon_name(m_item),
				&error);
		g_strfreev(argv);
	}

	if (G_UNLIKELY(!result))
	{
		xfce_dialog_show_error(nullptr, error, _("Failed to execute command \"%s\"."), command);
		g_error_free(error);
	}

	g_free(command);
}

//-----------------------------------------------------------------------------

void Launcher::run(GdkScreen* screen, DesktopAction* action) const
{
	// Expand the field codes
	const gchar* string = action->get_command();
	if (xfce_str_is_empty(string))
	{
		return;
	}

	gchar* uri = garcon_menu_item_get_uri(m_item);
	gchar* command = xfce_expand_desktop_entry_field_codes(string,
			nullptr,
			action->get_icon(),
			action->get_name(),
			uri,
			false);
	g_free(uri);

	// Parse and spawn command
	gchar** argv;
	bool result = false;
	GError* error = nullptr;
	if (g_shell_parse_argv(command, nullptr, &argv, &error))
	{
		result = xfce_spawn_on_screen(screen,
				garcon_menu_item_get_path(m_item),
				argv, nullptr, g_spawnFlags,
				garcon_menu_item_supports_startup_notification(m_item),
				gtk_get_current_event_time(),
				action->get_icon(),
				&error);
		g_strfreev(argv);
	}

	if (G_UNLIKELY(!result))
	{
		xfce_dialog_show_error(nullptr, error, _("Failed to execute command \"%s\"."), command);
		g_error_free(error);
	}

	g_free(command);
}

//-----------------------------------------------------------------------------

unsigned int Launcher::search(const Query& query)
{
	// Prioritize matches in favorites and recent, then favories, and then recent
	const unsigned int flags = 3 - m_search_flags;

	// Sort matches in names first
	unsigned int match = query.match(m_search_name);
	if (match != UINT_MAX)
	{
		return match | flags | 0x400;
	}

	match = query.match(m_search_generic_name);
	if (match != UINT_MAX)
	{
		return match | flags | 0x800;
	}

	// Sort matches in comments next
	match = query.match(m_search_comment);
	if (match != UINT_MAX)
	{
		return match | flags | 0x1000;
	}

	// Sort matches in keywords next
	for (const auto& keyword : m_search_keywords)
	{
		match = query.match(keyword);
		if (match != UINT_MAX)
		{
			return match | flags | 0x2000;
		}
	}

	// Sort matches in executables last
	match = query.match(m_search_command);
	if (match != UINT_MAX)
	{
		return match | flags | 0x4000;
	}

	return UINT_MAX;
}

//-----------------------------------------------------------------------------

void Launcher::set_flag(SearchFlag flag, bool enabled)
{
	if (enabled)
	{
		m_search_flags |= flag;
	}
	else
	{
		m_search_flags &= ~flag;
	}
}

//-----------------------------------------------------------------------------
