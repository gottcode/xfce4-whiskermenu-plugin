/*
 * Copyright (C) 2013, 2016 Graeme Gott <graeme@gottcode.org>
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

#include "resizer-widget.h"

#include "settings.h"
#include "slot.h"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

ResizerWidget::ResizerWidget(GtkWindow* window) :
	m_window(window),
	m_cursor(NULL),
	m_shape(3)
{
	m_drawing = gtk_drawing_area_new();
	gtk_widget_set_halign(m_drawing, GTK_ALIGN_END);
	gtk_widget_set_valign(m_drawing, GTK_ALIGN_START);
	gtk_widget_set_size_request(m_drawing, 10, 10);
	gtk_widget_add_events(m_drawing, GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

	g_signal_connect_slot(m_drawing, "button-press-event", &ResizerWidget::on_button_press_event, this);
	g_signal_connect_slot(m_drawing, "enter-notify-event", &ResizerWidget::on_enter_notify_event, this);
	g_signal_connect_slot(m_drawing, "leave-notify-event", &ResizerWidget::on_leave_notify_event, this);
	g_signal_connect_slot(m_drawing, "draw", &ResizerWidget::on_draw_event, this);

	set_corner(TopRight);
}

//-----------------------------------------------------------------------------

ResizerWidget::~ResizerWidget()
{
	if (m_cursor)
	{
		g_object_unref(G_OBJECT(m_cursor));
	}
}

//-----------------------------------------------------------------------------

void ResizerWidget::set_corner(Corner corner)
{
	static const GdkPoint bottomleft[] = { {10,10}, {0,10}, {0,0} };
	static const GdkPoint topleft[] = { {10,0}, {0,10}, {0,0} };
	static const GdkPoint bottomright[] = { {10,10}, {0,10}, {10,0} };
	static const GdkPoint topright[] = { {10,0}, {10,10}, {0,0} };

	GdkCursorType type;
	switch (corner)
	{
	case BottomLeft:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_START);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_END);
		m_shape.assign(bottomleft, bottomleft + 3);
		m_edge = GDK_WINDOW_EDGE_SOUTH_WEST;
		type = GDK_BOTTOM_LEFT_CORNER;
		break;
	case TopLeft:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_START);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_START);
		m_shape.assign(topleft, topleft + 3);
		m_edge = GDK_WINDOW_EDGE_NORTH_WEST;
		type = GDK_TOP_LEFT_CORNER;
		break;
	case BottomRight:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_END);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_END);
		m_shape.assign(bottomright, bottomright + 3);
		m_edge = GDK_WINDOW_EDGE_SOUTH_EAST;
		type = GDK_BOTTOM_RIGHT_CORNER;
		break;
	case TopRight:
	default:
		gtk_widget_set_halign(m_drawing, GTK_ALIGN_END);
		gtk_widget_set_valign(m_drawing, GTK_ALIGN_START);
		m_shape.assign(topright, topright + 3);
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

gboolean ResizerWidget::on_button_press_event(GtkWidget*, GdkEvent* event)
{
	GdkEventButton* event_button = reinterpret_cast<GdkEventButton*>(event);
	gtk_window_begin_resize_drag(m_window,
			m_edge,
			event_button->button,
			event_button->x_root,
			event_button->y_root,
			event_button->time);
	return true;
}

//-----------------------------------------------------------------------------

gboolean ResizerWidget::on_enter_notify_event(GtkWidget* widget, GdkEvent*)
{
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_window_set_cursor(window, m_cursor);
	return false;
}

//-----------------------------------------------------------------------------

gboolean ResizerWidget::on_leave_notify_event(GtkWidget* widget, GdkEvent*)
{
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_window_set_cursor(window, NULL);
	return false;
}

//-----------------------------------------------------------------------------

gboolean ResizerWidget::on_draw_event(GtkWidget* widget, cairo_t* cr)
{
	GdkRGBA color;
	GtkStyleContext* context = gtk_widget_get_style_context(widget);
	gtk_style_context_get_color(context, gtk_style_context_get_state(context), &color);
	gdk_cairo_set_source_rgba(cr, &color);

	cairo_move_to(cr, m_shape.back().x, m_shape.back().y);
	for (std::vector<GdkPoint>::const_iterator point = m_shape.begin(), end = m_shape.end(); point != end; ++point)
	{
		cairo_line_to(cr, point->x, point->y);
	}
	cairo_fill(cr);

	return true;
}

//-----------------------------------------------------------------------------
