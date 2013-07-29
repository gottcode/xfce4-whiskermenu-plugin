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


#ifndef WHISKERMENU_ELEMENT_HPP
#define WHISKERMENU_ELEMENT_HPP

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class Element
{
public:
	Element() :
		m_icon(NULL),
		m_text(NULL),
		m_sort_key(NULL)
	{
	}

	virtual ~Element()
	{
		g_free(m_icon);
		g_free(m_text);
		g_free(m_sort_key);
	}

	virtual int get_type() const = 0;

	const gchar* get_icon() const
	{
		return m_icon;
	}

	const gchar* get_text() const
	{
		return m_text;
	}

	static bool less_than(const Element* lhs, const Element* rhs)
	{
		return g_strcmp0(lhs->m_sort_key, rhs->m_sort_key) < 0;
	}

protected:
	void set_icon(const gchar* icon)
	{
		m_icon = g_strdup(icon);
	}

	void set_icon(gchar* icon)
	{
		m_icon = icon;
	}

	void set_text(const gchar* text)
	{
		m_text = g_strdup(text);
		m_sort_key = g_utf8_collate_key(m_text, -1);
	}

	void set_text(gchar* text)
	{
		m_text = text;
		m_sort_key = g_utf8_collate_key(m_text, -1);
	}

private:
	gchar* m_icon;
	gchar* m_text;
	gchar* m_sort_key;
};

}

#endif // WHISKERMENU_ELEMENT_HPP
