/*
 * Copyright (C) 2013, 2016, 2018, 2020 Graeme Gott <graeme@gottcode.org>
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

#include "image-menu-item.h"
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

Command::Command(const gchar* icon, const gchar* text, const gchar* command, const gchar* error_text, const gchar* confirm_question, const gchar* confirm_status) :
	m_button(NULL),
	m_menuitem(NULL),
	m_icon(g_strdup(icon)),
	m_mnemonic(g_strdup(text)),
	m_command(g_strdup(command)),
	m_error_text(g_strdup(error_text)),
	m_status(WHISKERMENU_COMMAND_UNCHECKED),
	m_shown(true),
	m_timeout_details({NULL, g_strdup(confirm_question), g_strdup(confirm_status), 0})
{
	std::string mnemonic(text ? text : "");
	for (std::string::size_type i = 0, length = mnemonic.length(); i < length; ++i)
	{
		if (mnemonic[i] == '_')
		{
			mnemonic.erase(i, 1);
			--length;
			--i;
		}
	}
	m_text = g_strdup(mnemonic.c_str());

	check();
}

//-----------------------------------------------------------------------------

Command::~Command()
{
	if (m_button)
	{
		gtk_widget_destroy(m_button);
		g_object_unref(m_button);
	}
	if (m_menuitem)
	{
		gtk_widget_destroy(m_menuitem);
		g_object_unref(m_menuitem);
	}

	g_free(m_icon);
	g_free(m_mnemonic);
	g_free(m_text);
	g_free(m_command);
	g_free(m_error_text);
	g_free(m_timeout_details.question);
	g_free(m_timeout_details.status);
}

//-----------------------------------------------------------------------------

GtkWidget* Command::get_button()
{
	if (m_button)
	{
		return m_button;
	}

	m_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(m_button), GTK_RELIEF_NONE);
	gtk_widget_set_tooltip_text(m_button, m_text);
	g_signal_connect_slot<GtkButton*>(m_button, "clicked", &Command::activate, this, true);

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

	m_menuitem = whiskermenu_image_menu_item_new_with_mnemonic(m_icon, m_mnemonic);
	g_signal_connect_slot<GtkMenuItem*>(m_menuitem, "activate", &Command::activate, this);

	gtk_widget_set_visible(m_menuitem, m_shown);
	gtk_widget_set_sensitive(m_menuitem, m_status == WHISKERMENU_COMMAND_VALID);

	g_object_ref_sink(m_menuitem);

	return m_menuitem;
}

//-----------------------------------------------------------------------------

void Command::set(const gchar* command)
{
	if (g_strcmp0(command, m_command) == 0)
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
	if (wm_settings->confirm_session_command
			&& m_timeout_details.question
			&& m_timeout_details.status
			&& !confirm())
	{
		return;
	}

	GError* error = NULL;
	if (!g_spawn_command_line_async(m_command, &error))
	{
		xfce_dialog_show_error(NULL, error, m_error_text, NULL);
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------

// Adapted from https://git.xfce.org/xfce/xfce4-panel/tree/plugins/actions/actions.c
bool Command::confirm()
{
	// Create dialog
	m_timeout_details.dialog = gtk_message_dialog_new(NULL, GtkDialogFlags(0),
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_CANCEL,
			"%s", m_timeout_details.question);
	GtkDialog* dialog = GTK_DIALOG(m_timeout_details.dialog);

	GtkWindow* window = GTK_WINDOW(m_timeout_details.dialog);
	gtk_window_set_keep_above(window, true);
	gtk_window_stick(window);
	gtk_window_set_skip_taskbar_hint(window, true);
	gtk_window_set_title(window, m_text);

	// Create accept button
	GtkWidget* button = gtk_dialog_add_button(dialog, m_mnemonic, GTK_RESPONSE_ACCEPT);
	GtkWidget* image = gtk_image_new_from_icon_name(m_icon, GTK_ICON_SIZE_DIALOG);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_TOP);
	gtk_dialog_set_default_response(dialog, GTK_RESPONSE_ACCEPT);

	// Add icon to cancel button
	button = gtk_dialog_get_widget_for_response(dialog, GTK_RESPONSE_CANCEL);
	if (GTK_IS_BUTTON(button))
	{
		image = gtk_image_new_from_icon_name("gtk-cancel", GTK_ICON_SIZE_DIALOG);
		gtk_button_set_image(GTK_BUTTON(button), image);
		gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_TOP);
	}

	// Run dialog
	m_timeout_details.time_left = 60;
	guint timeout_id = g_timeout_add(1000, &Command::confirm_countdown, &m_timeout_details);
	confirm_countdown(&m_timeout_details);

	gint result = gtk_dialog_run(dialog);

	g_source_remove(timeout_id);
	gtk_widget_destroy(m_timeout_details.dialog);
	m_timeout_details.dialog = NULL;

	return result == GTK_RESPONSE_ACCEPT;
}

//-----------------------------------------------------------------------------

gboolean Command::confirm_countdown(gpointer data)
{
	TimeoutDetails* details = static_cast<TimeoutDetails*>(data);

	if (details->time_left == 0)
	{
		gtk_dialog_response(GTK_DIALOG(details->dialog), GTK_RESPONSE_ACCEPT);
	}
	else
	{
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(details->dialog),
				details->status, details->time_left);
	}

	return --details->time_left >= 0;
}

//-----------------------------------------------------------------------------
