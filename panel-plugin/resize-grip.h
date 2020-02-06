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

#ifndef WHISKERMENU_RESIZE_GRIP_H
#define WHISKERMENU_RESIZE_GRIP_H

#include <vector>

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class ResizeGrip
{
public:
	explicit ResizeGrip(GtkWindow* window);
	~ResizeGrip();

	ResizeGrip(const ResizeGrip&) = delete;
	ResizeGrip(ResizeGrip&&) = delete;
	ResizeGrip& operator=(const ResizeGrip&) = delete;
	ResizeGrip& operator=(ResizeGrip&&) = delete;

	GtkWidget* get_widget() const
	{
		return m_drawing;
	}

	enum Corner
	{
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};
	void set_corner(Corner corner);

private:
	gboolean on_button_press_event(GtkWidget*, GdkEvent* event);
	gboolean on_enter_notify_event(GtkWidget* widget, GdkEvent*);
	gboolean on_leave_notify_event(GtkWidget* widget, GdkEvent*);
	gboolean on_draw_event(GtkWidget* widget, cairo_t* cr);

private:
	GtkWindow* m_window;
	GtkWidget* m_drawing;
	GdkCursor* m_cursor;
	GdkWindowEdge m_edge;
	std::vector<GdkPoint> m_shape;
};

}

#endif // WHISKERMENU_RESIZE_GRIP_H
