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


#ifndef WHISKERMENU_CATEGORY_HPP
#define WHISKERMENU_CATEGORY_HPP

#include <algorithm>
#include <vector>

extern "C"
{
#include <garcon/garcon.h>
}

namespace WhiskerMenu
{

class Launcher;
class SectionButton;

class Category
{
public:
	explicit Category(GarconMenuDirectory* directory);
	~Category();

	SectionButton* get_button() const
	{
		return m_button;
	}

	bool contains(Launcher* launcher) const
	{
		return std::find(m_items.begin(), m_items.end(), launcher) != m_items.end();
	}

	bool empty() const
	{
		return m_items.empty();
	}

	void push_back(Launcher* launcher)
	{
		m_items.push_back(launcher);
	}

private:
	std::vector<Launcher*> m_items;
	SectionButton* m_button;
};

}

#endif // WHISKERMENU_CATEGORY_HPP
