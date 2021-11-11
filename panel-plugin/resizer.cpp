/*
 * Copyright (C) 2013-2021 Graeme Gott <graeme@gottcode.org>
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
	m_cursor(nullptr)
{
	m_drawing = gtk_drawing_area_new();
	gtk_widget_set_size_request(m_drawing, 6, 6);
	gtk_widget_add_events(m_drawing, GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

	connect(m_drawing, "button-press-event",
		[this](GtkWidget*, GdkEvent* event) -> gboolean
		{
			m_window->set_child_has_focus();

			GdkEventButton* event_button = reinterpret_cast<GdkEventButton*>(event);
			gtk_window_begin_resize_drag(GTK_WINDOW(m_window->get_widget()),
					m_edge,
					event_button->button,
					event_button->x_root,
					event_button->y_root,
					event_button->time);
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
		m_edge = GDK_WINDOW_EDGE_SOUTH_WEST;
		type = "nesw-resize";
		break;

	case Bottom:
		m_edge = GDK_WINDOW_EDGE_SOUTH;
		type = "ns-resize";
		break;

	case BottomRight:
		m_edge = GDK_WINDOW_EDGE_SOUTH_EAST;
		type = "nwse-resize";
		break;

	case Left:
		m_edge = GDK_WINDOW_EDGE_WEST;
		type = "ew-resize";
		break;

	case Right:
		m_edge = GDK_WINDOW_EDGE_EAST;
		type = "ew-resize";
		break;

	case TopLeft:
		m_edge = GDK_WINDOW_EDGE_NORTH_WEST;
		type = "nwse-resize";
		break;

	case Top:
		m_edge = GDK_WINDOW_EDGE_NORTH;
		type = "ns-resize";
		break;

	case TopRight:
	default:
		m_edge = GDK_WINDOW_EDGE_NORTH_EAST;
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
