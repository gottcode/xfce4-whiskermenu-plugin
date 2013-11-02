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

#include "command-button.h"

extern "C"
{
#include <libxfce4ui/libxfce4ui.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

CommandButton::CommandButton(const gchar* icon, const gchar* text, std::string& command, const std::string& error_text) :
	m_command(command),
	m_error_text(error_text),
	m_status(Unchecked)
{
	m_button = GTK_BUTTON(gtk_button_new());
	gtk_button_set_relief(m_button, GTK_RELIEF_NONE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(m_button), text);
	g_signal_connect(m_button, "clicked", G_CALLBACK(CommandButton::clicked_slot), this);

	GtkWidget* image = gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(image));
}

//-----------------------------------------------------------------------------

CommandButton::~CommandButton()
{
	gtk_widget_destroy(GTK_WIDGET(m_button));
}

//-----------------------------------------------------------------------------

void CommandButton::set_command(const std::string& command)
{
	m_command = command;
	m_status = Unchecked;
}

//-----------------------------------------------------------------------------

void CommandButton::check()
{
	if (m_status == Unchecked)
	{
		gchar* path = g_find_program_in_path(m_command.c_str());
		m_status = path ? Valid : Invalid;
		g_free(path);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(m_button), m_status == Valid);
}

//-----------------------------------------------------------------------------

void CommandButton::clicked()
{
	GError* error = NULL;
	if (g_spawn_command_line_async(m_command.c_str(), &error) == false)
	{
		xfce_dialog_show_error(NULL, error, m_error_text.c_str());
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------
