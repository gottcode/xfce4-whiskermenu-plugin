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

#include "command.h"

#include "settings.h"
#include "slot.h"

#include <string>

#include <libxfce4ui/libxfce4ui.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

enum
{
	WHISKERMENU_COMMAND_UNCHECKED = -1,
	WHISKERMENU_COMMAND_INVALID,
	WHISKERMENU_COMMAND_VALID
};

//-----------------------------------------------------------------------------

Command::Command(const gchar* icon, const gchar* text, const gchar* command, const gchar* error_text) :
	m_button(NULL),
	m_menuitem(NULL),
	m_icon(g_strdup(icon)),
	m_text(g_strdup(text)),
	m_command(g_strdup(command)),
	m_error_text(g_strdup(error_text)),
	m_status(WHISKERMENU_COMMAND_UNCHECKED),
	m_shown(true)
{
	check();
}

//-----------------------------------------------------------------------------

Command::~Command()
{
	if (m_button)
	{
		g_object_unref(m_button);
	}
	if (m_menuitem)
	{
		g_object_unref(m_menuitem);
	}

	g_free(m_icon);
	g_free(m_text);
	g_free(m_command);
	g_free(m_error_text);
}

//-----------------------------------------------------------------------------

GtkWidget* Command::get_button()
{
	if (m_button)
	{
		return m_button;
	}

	std::string tooltip(m_text ? m_text : "");
	for (std::string::size_type i = 0, length = tooltip.length(); i < length; ++i)
	{
		if (tooltip[i] == '_')
		{
			tooltip.erase(i, 1);
			--length;
			--i;
		}
	}

	m_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(m_button), GTK_RELIEF_NONE);
	gtk_widget_set_tooltip_text(m_button, tooltip.c_str());
	g_signal_connect_slot<GtkButton*>(m_button, "clicked", &Command::activate, this);

	GtkWidget* image = gtk_image_new_from_icon_name(m_icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(image));

	gtk_widget_set_visible(m_button, m_shown);
	gtk_widget_set_sensitive(m_button, m_status == WHISKERMENU_COMMAND_VALID);

	g_object_ref_sink(m_button);

	return m_button;
}

//-----------------------------------------------------------------------------

GtkWidget* Command::get_menuitem()
{
	if (m_menuitem)
	{
		return m_menuitem;
	}

	m_menuitem = gtk_image_menu_item_new_with_mnemonic(m_text);
	GtkWidget* image = gtk_image_new_from_icon_name(m_icon, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(m_menuitem), image);
	g_signal_connect_slot<GtkMenuItem*>(m_menuitem, "activate", &Command::activate, this);

	gtk_widget_set_visible(m_menuitem, m_shown);
	gtk_widget_set_sensitive(m_menuitem, m_status == WHISKERMENU_COMMAND_VALID);

	g_object_ref_sink(m_menuitem);

	return m_menuitem;
}

//-----------------------------------------------------------------------------

void Command::set(const gchar* command)
{
	if (command == m_command)
	{
		return;
	}

	g_free(m_command);
	m_command = g_strdup(command);
	m_status = WHISKERMENU_COMMAND_UNCHECKED;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void Command::set_shown(bool shown)
{
	if (shown == m_shown)
	{
		return;
	}

	m_shown = shown;
	wm_settings->set_modified();

	if (m_button)
	{
		gtk_widget_set_visible(m_button, m_shown);
	}
	if (m_menuitem)
	{
		gtk_widget_set_visible(m_menuitem, m_shown);
	}
}

//-----------------------------------------------------------------------------

void Command::check()
{
	if (m_status == WHISKERMENU_COMMAND_UNCHECKED)
	{
		gchar** argv;
		if (g_shell_parse_argv(m_command, NULL, &argv, NULL))
		{
			gchar* path = g_find_program_in_path(argv[0]);
			m_status = path ? WHISKERMENU_COMMAND_VALID : WHISKERMENU_COMMAND_INVALID;
			g_free(path);
			g_strfreev(argv);
		}
		else
		{
			m_status = WHISKERMENU_COMMAND_INVALID;
		}
	}

	if (m_button)
	{
		gtk_widget_set_visible(m_button, m_shown);
		gtk_widget_set_sensitive(m_button, m_status == WHISKERMENU_COMMAND_VALID);
	}
	if (m_menuitem)
	{
		gtk_widget_set_visible(m_menuitem, m_shown);
		gtk_widget_set_sensitive(m_menuitem, m_status == WHISKERMENU_COMMAND_VALID);
	}
}

//-----------------------------------------------------------------------------

void Command::activate()
{
	GError* error = NULL;
	if (g_spawn_command_line_async(m_command, &error) == false)
	{
		xfce_dialog_show_error(NULL, error, m_error_text, NULL);
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------
