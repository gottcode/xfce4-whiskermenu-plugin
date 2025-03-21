/*
 * Copyright (C) 2013-2025 Graeme Gott <graeme@gottcode.org>
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

#include <libxfce4ui/libxfce4ui.h>
#include <glib/gstdio.h>

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

Launcher::Launcher(GarconMenuItem* item) :
	m_item(item)
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
	if (wm_settings->launcher_show_description && (wm_settings->view_mode != Settings::ViewAsIcons))
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
	GList* keywords = garcon_menu_item_get_keywords(m_item);
	for (GList* i = keywords; i; i = i->next)
	{
		const gchar* keyword = static_cast<gchar*>(i->data);
		if (!xfce_str_is_empty(keyword) && g_utf8_validate(keyword, -1, nullptr))
		{
			m_search_keywords.push_back(normalize(keyword));
		}
	}

	// Create search text for command
	const gchar* command = garcon_menu_item_get_command(m_item);
	if (!xfce_str_is_empty(command) && g_utf8_validate(command, -1, nullptr))
	{
		m_search_command = normalize(command);
	}

	// Fetch desktop actions
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

bool Launcher::has_auto_start() const
{
	const std::string filename = std::string("autostart/") + get_desktop_id();

	// Check if autostart launcher exists
	gchar* path = xfce_resource_lookup(XFCE_RESOURCE_CONFIG, filename.c_str());
	if (!path)
	{
		return false;
	}
	g_free(path);

	// Check if launcher is hidden or invalid
	XfceRc* rc = xfce_rc_config_open(XFCE_RESOURCE_CONFIG, filename.c_str(), true);
	if (!rc)
	{
		return false;
	}

	xfce_rc_set_group(rc, "Desktop Entry");
	const bool hidden = xfce_rc_read_bool_entry(rc, "Hidden", false);
	const bool invalid = xfce_str_is_empty(xfce_rc_read_entry(rc, "Exec", nullptr));

	xfce_rc_close(rc);

	return !hidden && !invalid;
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

	if (xfce_dialog_confirm(nullptr, nullptr, _("Hide Application"), message,
		_("Are you sure you want to hide \"%s\"?"), m_display_name))
	{
		GFile* source = garcon_menu_item_get_file(m_item);
		GFile* dest = g_file_new_for_path(path);
		if (!g_file_equal(source, dest))
		{
			g_file_copy(source, dest, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, nullptr);
		}
		g_object_unref(source);
		g_object_unref(dest);

		XfceRc* rc = xfce_rc_config_open(XFCE_RESOURCE_DATA, relpath, false);
		xfce_rc_set_group(rc, G_KEY_FILE_DESKTOP_GROUP);
		xfce_rc_write_bool_entry(rc, G_KEY_FILE_DESKTOP_KEY_HIDDEN, true);
		xfce_rc_close(rc);
	}

	g_free(message);
	g_free(path);
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
	spawn(screen, command,
			garcon_menu_item_get_path(m_item),
			garcon_menu_item_supports_startup_notification(m_item),
			garcon_menu_item_get_icon_name(m_item));

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
	spawn(screen, command,
			garcon_menu_item_get_path(m_item),
			garcon_menu_item_supports_startup_notification(m_item),
			action->get_icon());

	g_free(command);
}

//-----------------------------------------------------------------------------

unsigned int Launcher::search(const Query& query)
{
	// Sort matches in names first
	unsigned int match = query.match(m_search_name);
	if (match != UINT_MAX)
	{
		return match | 0x400;
	}

	match = query.match_as_characters(m_search_name);
	if (match != UINT_MAX)
	{
		return match | 0x400;
	}

	match = query.match(m_search_generic_name);
	if (match != UINT_MAX)
	{
		return match | 0x800;
	}

	// Sort matches in comments next
	match = query.match(m_search_comment);
	if (match != UINT_MAX)
	{
		return match | 0x1000;
	}

	// Sort matches in keywords next
	for (const auto& keyword : m_search_keywords)
	{
		match = query.match(keyword);
		if (match != UINT_MAX)
		{
			return match | 0x2000;
		}
	}

	// Sort matches in executables last
	match = query.match(m_search_command);
	if (match != UINT_MAX)
	{
		return match | 0x4000;
	}

	return UINT_MAX;
}

//-----------------------------------------------------------------------------

void Launcher::set_auto_start(bool enabled)
{
	// Fetch autostart path for launcher
	const std::string filename = std::string("autostart/") + get_desktop_id();
	gchar* path = xfce_resource_save_location(XFCE_RESOURCE_CONFIG, filename.c_str(), true);

	// Always remove launcher from autostart directory
	g_remove(path);

	if (enabled)
	{
		// Copy launcher to autostart directory
		GFile* source = get_file();
		GFile* dest = g_file_new_for_path(path);
		g_file_copy(source, dest, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, nullptr);
		g_object_unref(source);
		g_object_unref(dest);
	}
	else if (has_auto_start())
	{
		// Disable global autostart
		XfceRc* rc = xfce_rc_config_open(XFCE_RESOURCE_CONFIG, filename.c_str(), false);
		if (rc)
		{
			xfce_rc_set_group(rc, "Desktop Entry");
			xfce_rc_write_bool_entry(rc, "Hidden", true);
			xfce_rc_close(rc);
		}
	}

	// Clean up
	g_free(path);
}

//-----------------------------------------------------------------------------
