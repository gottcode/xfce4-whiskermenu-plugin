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
	g_signal_connect_slot(m_drawing, "button-press-event", &Resizer::on_button_press_event, this);
	g_signal_connect_slot(m_drawing, "enter-notify-event", &Resizer::on_enter_notify_event, this);
	g_signal_connect_slot(m_drawing, "leave-notify-event", &Resizer::on_leave_notify_event, this);

	GdkCursorType type;
	switch (edge)
	{
	case BottomLeft:
		m_edge = GDK_WINDOW_EDGE_SOUTH_WEST;
		type = GDK_BOTTOM_LEFT_CORNER;
		break;

	case Bottom:
		m_edge = GDK_WINDOW_EDGE_SOUTH;
		type = GDK_BOTTOM_SIDE;
		break;

	case BottomRight:
		m_edge = GDK_WINDOW_EDGE_SOUTH_EAST;
		type = GDK_BOTTOM_RIGHT_CORNER;
		break;

	case Left:
		m_edge = GDK_WINDOW_EDGE_WEST;
		type = GDK_LEFT_SIDE;
		break;

	case Right:
		m_edge = GDK_WINDOW_EDGE_EAST;
		type = GDK_RIGHT_SIDE;
		break;

	case TopLeft:
		m_edge = GDK_WINDOW_EDGE_NORTH_WEST;
		type = GDK_TOP_LEFT_CORNER;
		break;

	case Top:
		m_edge = GDK_WINDOW_EDGE_NORTH;
		type = GDK_TOP_SIDE;
		break;

	case TopRight:
	default:
		m_edge = GDK_WINDOW_EDGE_NORTH_EAST;
		type = GDK_TOP_RIGHT_CORNER;
		break;
	}
	m_cursor = gdk_cursor_new_for_display(gtk_widget_get_display(m_drawing), type);
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

gboolean Resizer::on_button_press_event(GtkWidget*, GdkEvent* event)
{
	m_window->set_child_has_focus();

	GtkWindow* window = GTK_WINDOW(m_window->get_widget());

	GdkEventButton* event_button = reinterpret_cast<GdkEventButton*>(event);
	gtk_window_begin_resize_drag(window,
			m_edge,
			event_button->button,
			event_button->x_root,
			event_button->y_root,
			event_button->time);
	return GDK_EVENT_STOP;
}

//-----------------------------------------------------------------------------

gboolean Resizer::on_enter_notify_event(GtkWidget* widget, GdkEvent*)
{
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_window_set_cursor(window, m_cursor);
	return GDK_EVENT_PROPAGATE;
}

//-----------------------------------------------------------------------------

gboolean Resizer::on_leave_notify_event(GtkWidget* widget, GdkEvent*)
{
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_window_set_cursor(window, nullptr);
	return GDK_EVENT_PROPAGATE;
}

//-----------------------------------------------------------------------------
