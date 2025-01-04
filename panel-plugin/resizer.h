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

#ifndef WHISKERMENU_RESIZER_H
#define WHISKERMENU_RESIZER_H

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class Window;

class Resizer
{
public:
	enum Edge
	{
		TopLeft = 0,
		Top,
		TopRight,
		Left,
		Right,
		BottomLeft,
		Bottom,
		BottomRight
	};

	explicit Resizer(Edge edge, Window* window);
	~Resizer();

	Resizer(const Resizer&) = delete;
	Resizer(Resizer&&) = delete;
	Resizer& operator=(const Resizer&) = delete;
	Resizer& operator=(Resizer&&) = delete;

	GtkWidget* get_widget() const
	{
		return m_drawing;
	}

	void cancel()
	{
		m_pressed = false;
	}

private:
	Window* m_window;
	GtkWidget* m_drawing;
	GdkCursor* m_cursor;
	double m_x;
	double m_y;
	int m_delta_x;
	int m_delta_y;
	int m_delta_width;
	int m_delta_height;
	bool m_pressed;
};

}

#endif // WHISKERMENU_RESIZER_H
