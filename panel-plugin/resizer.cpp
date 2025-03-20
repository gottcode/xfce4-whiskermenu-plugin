/*
 * Copyright (C) 2021-2025 Graeme Gott <graeme@gottcode.org>
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

#include "resizer.h"

#include "slot.h"
#include "window.h"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

Resizer::Resizer(Edge edge, Window* window) :
	m_window(window),
	m_cursor(nullptr),
	m_x(0),
	m_y(0),
	m_delta_x(0),
	m_delta_y(0),
	m_delta_width(0),
	m_delta_height(0),
	m_pressed(false)
{
	m_drawing = gtk_drawing_area_new();
	gtk_widget_set_size_request(m_drawing, 6, 6);
	gtk_widget_add_events(m_drawing, GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_ENTER_NOTIFY_MASK
			| GDK_LEAVE_NOTIFY_MASK);

	connect(m_drawing, "button-press-event",
		[this](GtkWidget*, GdkEvent* event) -> gboolean
		{
			GdkEventButton* event_button = reinterpret_cast<GdkEventButton*>(event);
			if (event_button->button != 1)
			{
				return GDK_EVENT_STOP;
			}
			m_pressed = true;

			m_x = event_button->x;
			m_y = event_button->y;

			m_window->resize_start();

			return GDK_EVENT_STOP;
		});

	connect(m_drawing, "button-release-event",
		[this](GtkWidget*, GdkEvent* event) -> gboolean
		{
			GdkEventButton* event_button = reinterpret_cast<GdkEventButton*>(event);
			if (event_button->button != 1)
			{
				return GDK_EVENT_STOP;
			}
			m_pressed = false;

			m_window->resize_end();

			return GDK_EVENT_STOP;
		});

	connect(m_drawing, "motion-notify-event",
		[this](GtkWidget*, GdkEvent* event) -> gboolean
		{
			if (!m_pressed)
			{
				return GDK_EVENT_STOP;
			}

			GdkEventButton* event_button = reinterpret_cast<GdkEventButton*>(event);
			const int delta_x = event_button->x - m_x;
			const int delta_y = event_button->y - m_y;

			m_window->resize(delta_x * m_delta_x, delta_y * m_delta_y, delta_x * m_delta_width, delta_y * m_delta_height);

			return GDK_EVENT_STOP;
		});

	connect(m_drawing, "enter-notify-event",
		[this](GtkWidget* widget, GdkEvent*) -> gboolean
		{
			gdk_window_set_cursor(gtk_widget_get_window(widget), m_cursor);
			return GDK_EVENT_PROPAGATE;
		});

	connect(m_drawing, "leave-notify-event",
		[](GtkWidget* widget, GdkEvent*) -> gboolean
		{
			gdk_window_set_cursor(gtk_widget_get_window(widget), nullptr);
			return GDK_EVENT_PROPAGATE;
		});

	const char* type = nullptr;
	switch (edge)
	{
	case BottomLeft:
		m_delta_x = 1;
		m_delta_width = -1;
		m_delta_height = 1;
		type = "nesw-resize";
		break;

	case Bottom:
		m_delta_height = 1;
		type = "ns-resize";
		break;

	case BottomRight:
		m_delta_width = 1;
		m_delta_height = 1;
		type = "nwse-resize";
		break;

	case Left:
		m_delta_x = 1;
		m_delta_width = -1;
		type = "ew-resize";
		break;

	case Right:
		m_delta_width = 1;
		type = "ew-resize";
		break;

	case TopLeft:
		m_delta_x = 1;
		m_delta_width = -1;
		m_delta_y = 1;
		m_delta_height = -1;
		type = "nwse-resize";
		break;

	case Top:
		m_delta_y = 1;
		m_delta_height = -1;
		type = "ns-resize";
		break;

	case TopRight:
	default:
		m_delta_y = 1;
		m_delta_height = -1;
		m_delta_width = 1;
		type = "nesw-resize";
		break;
	}
	m_cursor = gdk_cursor_new_from_name(gtk_widget_get_display(m_drawing), type);
}

//-----------------------------------------------------------------------------

Resizer::~Resizer()
{
	if (m_cursor)
	{
		g_object_unref(G_OBJECT(m_cursor));
	}
}

//-----------------------------------------------------------------------------
