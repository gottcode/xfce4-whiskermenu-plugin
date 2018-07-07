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

#include "resize-grip.h"

#include "settings.h"
#include "slot.h"
#include "window.h"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

ResizeGrip::ResizeGrip(Window* window) :
	m_window(window),
	m_cursor(nullptr),
	m_shape(3)
{
	m_drawing = gtk_drawing_area_new();
	gtk_widget_set_halign(m_drawing, GTK_ALIGN_END);
	gtk_widget_set_valign(m_drawing, GTK_ALIGN_START);
	gtk_widget_set_size_request(m_drawing, 10, 10);
	gtk_widget_add_events(m_drawing, GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

	g_signal_connect_slot(m_drawing, "button-press-event", &ResizeGrip::on_button_press_event, this);
	g_signal_connect_slot(m_drawing, "enter-notify-event", &ResizeGrip::on_enter_notify_event, this);
	g_signal_connect_slot(m_drawing, "leave-notify-event", &ResizeGrip::on_leave_notify_event, this);
	g_signal_connect_slot(m_drawing, "draw", &ResizeGrip::on_draw_event, this);

	set_corner(TopRight);
}

//-----------------------------------------------------------------------------

ResizeGrip::~ResizeGrip()
{
	if (m_cursor)
	{
		g_object_unref(G_OBJECT(m_cursor));
	}
}

//-----------------------------------------------------------------------------

void ResizeGrip::set_corner(Corner corner)
{
	GdkCursorType type;
	switch (corner)
	{
	case BottomLeft:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_START);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_END);
		m_shape = { {10,10}, {0,10}, {0,0} };
		m_edge = GDK_WINDOW_EDGE_SOUTH_WEST;
		type = GDK_BOTTOM_LEFT_CORNER;
		break;
	case TopLeft:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_START);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_START);
		m_shape = { {10,0}, {0,10}, {0,0} };
		m_edge = GDK_WINDOW_EDGE_NORTH_WEST;
		type = GDK_TOP_LEFT_CORNER;
		break;
	case BottomRight:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_END);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_END);
		m_shape = { {10,10}, {0,10}, {10,0} };
		m_edge = GDK_WINDOW_EDGE_SOUTH_EAST;
		type = GDK_BOTTOM_RIGHT_CORNER;
		break;
	case TopRight:
	default:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_END);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_START);
		m_shape = { {10,0}, {10,10}, {0,0} };
		m_edge = GDK_WINDOW_EDGE_NORTH_EAST;
		type = GDK_TOP_RIGHT_CORNER;
		break;
	}

	if (m_cursor)
	{
		g_object_unref(G_OBJECT(m_cursor));
	}
	m_cursor = gdk_cursor_new_for_display(gtk_widget_get_display(GTK_WIDGET(m_drawing)), type);
}

//-----------------------------------------------------------------------------

gboolean ResizeGrip::on_button_press_event(GtkWidget*, GdkEvent* event)
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

gboolean ResizeGrip::on_enter_notify_event(GtkWidget* widget, GdkEvent*)
{
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_window_set_cursor(window, m_cursor);
	return GDK_EVENT_PROPAGATE;
}

//-----------------------------------------------------------------------------

gboolean ResizeGrip::on_leave_notify_event(GtkWidget* widget, GdkEvent*)
{
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_window_set_cursor(window, nullptr);
	return GDK_EVENT_PROPAGATE;
}

//-----------------------------------------------------------------------------

gboolean ResizeGrip::on_draw_event(GtkWidget* widget, cairo_t* cr)
{
	GdkRGBA color;
	GtkStyleContext* context = gtk_widget_get_style_context(widget);
	gtk_style_context_get_color(context, gtk_style_context_get_state(context), &color);
	gdk_cairo_set_source_rgba(cr, &color);

	cairo_move_to(cr, m_shape.back().x, m_shape.back().y);
	for (const auto& point : m_shape)
	{
		cairo_line_to(cr, point.x, point.y);
	}
	cairo_fill(cr);

	return GDK_EVENT_STOP;
}

//-----------------------------------------------------------------------------
