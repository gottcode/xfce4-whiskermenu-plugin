/*
 * Copyright (C) 2013, 2014 Graeme Gott <graeme@gottcode.org>
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

#include <exo/exo.h>
#include <libxfce4ui/libxfce4ui.h>

using namespace WhiskerMenu;

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

static void replace_with_quoted_string(std::string& command, size_t& index, const gchar* unquoted)
{
	if (!exo_str_is_empty(unquoted))
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

static void replace_with_quoted_string(std::string& command, size_t& index, const char* prefix, const gchar* unquoted)
{
	if (!exo_str_is_empty(unquoted))
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

static void replace_with_quoted_string(std::string& command, size_t& index, gchar* unquoted)
{
	replace_with_quoted_string(command, index, unquoted);
	g_free(unquoted);
}

//-----------------------------------------------------------------------------

Launcher::Launcher(GarconMenuItem* item) :
	m_item(item)
{
	// Fetch icon
	const gchar* icon = garcon_menu_item_get_icon_name(m_item);
	if (G_LIKELY(icon))
	{
		if (!g_path_is_absolute(icon))
		{
			gchar* pos = g_strrstr(icon, ".");
			if (!pos)
			{
				set_icon(icon);
			}
			else
			{
				gchar* suffix = g_utf8_casefold(pos, -1);
				if ((strcmp(suffix, ".png") == 0)
						|| (strcmp(suffix, ".xpm") == 0)
						|| (strcmp(suffix, ".svg") == 0)
						|| (strcmp(suffix, ".svgz") == 0))
				{
					set_icon(g_strndup(icon, pos - icon));
				}
				else
				{
					set_icon(icon);
				}
				g_free(suffix);
			}
		}
		else
		{
			set_icon(icon);
		}
	}

	// Fetch text
	const gchar* name = garcon_menu_item_get_name(m_item);
	if (G_UNLIKELY(!name) || !g_utf8_validate(name, -1, NULL))
	{
		name = "";
	}

	const gchar* generic_name = garcon_menu_item_get_generic_name(m_item);
	if (G_UNLIKELY(!generic_name) || !g_utf8_validate(generic_name, -1, NULL))
	{
		generic_name = "";
	}

	// Create display text
	const gchar* direction = (gtk_widget_get_default_direction() != GTK_TEXT_DIR_RTL) ? "\342\200\216" : "\342\200\217";
	m_display_name = (wm_settings->launcher_show_name || exo_str_is_empty(generic_name)) ? name : generic_name;
	if (wm_settings->launcher_show_description)
	{
		const gchar* details = garcon_menu_item_get_comment(m_item);
		if (!details || !g_utf8_validate(details, -1, NULL))
		{
			details = generic_name;
		}
		set_text(g_markup_printf_escaped("%s<b>%s</b>\n%s%s", direction, m_display_name, direction, details));

		// Create search text for comment
		m_search_comment = normalize(details);
	}
	else
	{
		set_text(g_markup_printf_escaped("%s%s", direction, m_display_name));
	}

	// Create search text for display name
	m_search_name = normalize(m_display_name);

	// Create search text for command
	const gchar* command = garcon_menu_item_get_command(m_item);
	if (!exo_str_is_empty(command) && g_utf8_validate(command, -1, NULL))
	{
		m_search_command = normalize(command);
	}
}

//-----------------------------------------------------------------------------

Launcher::~Launcher()
{
}

//-----------------------------------------------------------------------------

void Launcher::run(GdkScreen* screen) const
{
	const gchar* string = garcon_menu_item_get_command(m_item);
	if (exo_str_is_empty(string))
	{
		return;
	}
	std::string command(string);

	if (garcon_menu_item_requires_terminal(m_item))
	{
		command.insert(0, "exo-open --launch TerminalEmulator ");
	}

	// Expand the field codes
	size_t length = command.length() - 1;
	for (size_t i = 0; i < length; ++i)
	{
		if (G_UNLIKELY(command[i] == '%'))
		{
			switch (command[i + 1])
			{
			case 'i':
				replace_with_quoted_string(command, i, "--icon ", garcon_menu_item_get_icon_name(m_item));
				length = command.length() - 1;
				break;

			case 'c':
				replace_with_quoted_string(command, i, garcon_menu_item_get_name(m_item));
				length = command.length() - 1;
				break;

			case 'k':
				replace_with_quoted_string(command, i, garcon_menu_item_get_uri(m_item));
				length = command.length() - 1;
				break;

			case '%':
				command.erase(i, 1);
				length = command.length() - 1;
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
				command.erase(i, 2);
				length = command.length() - 1;
				break;
			}
		}
	}

	// Parse and spawn command
	gchar** argv;
	gboolean result = false;
	GError* error = NULL;
	if (g_shell_parse_argv(command.c_str(), NULL, &argv, &error))
	{
		result = xfce_spawn_on_screen(screen,
				garcon_menu_item_get_path(m_item),
				argv, NULL, G_SPAWN_SEARCH_PATH,
				garcon_menu_item_supports_startup_notification(m_item),
				gtk_get_current_event_time(),
				garcon_menu_item_get_icon_name(m_item),
				&error);
		g_strfreev(argv);
	}

	if (G_UNLIKELY(!result))
	{
		xfce_dialog_show_error(NULL, error, _("Failed to execute command \"%s\"."), string);
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------

int Launcher::search(const Query& query)
{
	int match = query.match(m_search_name);
	if (match != G_MAXINT)
	{
		return match;
	}

	// Sort matches in executables after matches in names
	match = query.match(m_search_command);
	if (match != G_MAXINT)
	{
		match += 10;
		return match;
	}

	// Sort matches in comments after matches in names
	match = query.match(m_search_comment);
	if (match != G_MAXINT)
	{
		match += 20;
	}
	return match;
}

//-----------------------------------------------------------------------------
