// Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this library.  If not, see <http://www.gnu.org/licenses/>.


#ifndef WHISKERMENU_RESIZER_WIDGET_HPP
#define WHISKERMENU_RESIZER_WIDGET_HPP

#include <vector>

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class ResizerWidget
{
public:
	explicit ResizerWidget(GtkWindow* window);
	~ResizerWidget();

	GtkWidget* get_widget() const
	{
		return GTK_WIDGET(m_alignment);
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
	bool on_button_press_event(GdkEventButton* event);
	bool on_enter_notify_event(GtkWidget* widget);
	bool on_leave_notify_event(GtkWidget* widget);
	bool on_expose_event(GtkWidget* widget);

private:
	GtkWindow* m_window;
	GtkAlignment* m_alignment;
	GtkWidget* m_drawing;
	GdkCursor* m_cursor;
	GdkWindowEdge m_edge;
	std::vector<GdkPoint> m_shape;


private:
	static gboolean on_button_press_event_slot(GtkWidget*, GdkEventButton* event, ResizerWidget* obj)
	{
		return obj->on_button_press_event(event);
	}

	static gboolean on_enter_notify_event_slot(GtkWidget* widget, GdkEventCrossing*, ResizerWidget* obj)
	{
		return obj->on_enter_notify_event(widget);
	}

	static gboolean on_leave_notify_event_slot(GtkWidget* widget, GdkEventCrossing*, ResizerWidget* obj)
	{
		return obj->on_leave_notify_event(widget);
	}

	static gboolean on_expose_event_slot(GtkWidget* widget, GdkEventExpose*, ResizerWidget* obj)
	{
		return obj->on_expose_event(widget);
	}
};

}

#endif // WHISKERMENU_RESIZER_WIDGET_HPP
