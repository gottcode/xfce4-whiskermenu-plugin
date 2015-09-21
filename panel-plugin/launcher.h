/*
 * Copyright (C) 2013, 2015 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_LAUNCHER_H
#define WHISKERMENU_LAUNCHER_H

#include "element.h"

#include <string>

#include <garcon/garcon.h>

namespace WhiskerMenu
{

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

	gchar* get_uri() const
	{
		return garcon_menu_item_get_uri(m_item);
	}

	void run(GdkScreen* screen) const;

	guint search(const Query& query);

	enum SearchFlag
	{
		RecentFlag = 0x1,
		FavoriteFlag = 0x2
	};
	void set_flag(SearchFlag flag, bool enabled);

private:
	GarconMenuItem* m_item;
	const gchar* m_display_name;
	std::string m_search_name;
	std::string m_search_generic_name;
	std::string m_search_comment;
	std::string m_search_command;
	guint m_search_flags;
};

}

#endif // WHISKERMENU_LAUNCHER_H
