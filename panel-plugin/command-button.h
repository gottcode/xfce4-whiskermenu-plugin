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

#ifndef WHISKERMENU_COMMAND_BUTTON_H
#define WHISKERMENU_COMMAND_BUTTON_H

#include <string>

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class CommandButton
{
	enum Status
	{
		Unchecked = -1,
		Invalid,
		Valid
	};

public:
	CommandButton(const gchar* icon, const gchar* text, const std::string& command, const std::string& error_text);
	~CommandButton();

	GtkWidget* get_widget() const
	{
		return GTK_WIDGET(m_button);
	}

	std::string get_command() const
	{
		return m_command;
	}

	void set_command(const std::string& command);

	void check();

private:
	void clicked();

private:
	GtkButton* m_button;
	std::string m_command;
	std::string m_error_text;
	Status m_status;


private:
	static void clicked_slot(GtkButton*, CommandButton* obj)
	{
		obj->clicked();
	}
};

}

#endif // WHISKERMENU_COMMAND_BUTTON_H
