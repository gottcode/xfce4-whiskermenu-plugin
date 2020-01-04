/*
 * Copyright (C) 2014, 2016, 2020 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_PROFILE_PICTURE_H
#define WHISKERMENU_PROFILE_PICTURE_H

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class Window;

class ProfilePicture
{
public:
	ProfilePicture(Window* window);
	~ProfilePicture();

	GtkWidget* get_widget() const
	{
		return m_container;
	}

	void reset_tooltip();

private:
	void on_file_changed(GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type);
	void on_button_press_event();

private:
	Window* m_window;
	GtkWidget* m_container;
	GtkWidget* m_image;
	GFileMonitor* m_file_monitor;
};

}

#endif // WHISKERMENU_PROFILE_PICTURE_H
