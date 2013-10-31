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


#ifndef WHISKERMENU_LAUNCHER_H
#define WHISKERMENU_LAUNCHER_H

#include "element.h"

#include <string>

extern "C"
{
#include <garcon/garcon.h>
#include <gdk/gdk.h>
}

namespace WhiskerMenu
{

class Query;

class Launcher : public Element
{
public:
	explicit Launcher(GarconMenuItem* item);
	~Launcher();

	enum
	{
		Type = 2
	};
	int get_type() const
	{
		return Type;
	}

	const gchar* get_display_name() const
	{
		return m_display_name;
	}

	const gchar* get_desktop_id() const
	{
		return garcon_menu_item_get_desktop_id(m_item);
	}

	GFile* get_file() const
	{
		return garcon_menu_item_get_file(m_item);
	}

	void run(GdkScreen* screen) const;

	int search(const Query& query) const;

	static bool get_show_name();
	static bool get_show_description();
	static void set_show_name(bool show);
	static void set_show_description(bool show);

private:
	Launcher(const Launcher& launcher);
	Launcher& operator=(const Launcher& launcher);

private:
	GarconMenuItem* m_item;
	const gchar* m_display_name;
	std::string m_search_name;
	std::string m_search_comment;
	std::string m_search_command;
};

}

#endif // WHISKERMENU_LAUNCHER_H
