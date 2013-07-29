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

#include "element.hpp"

#include <vector>

extern "C"
{
#include <garcon/garcon.h>
}

namespace WhiskerMenu
{

class Launcher;
class SectionButton;

class Category : public Element
{
public:
	explicit Category(GarconMenuDirectory* directory);
	~Category();

	enum
	{
		Type = 1
	};
	int get_type() const
	{
		return Type;
	}

	SectionButton* get_button();

	GtkTreeModel* get_model();

	bool empty() const
	{
		return m_items.empty();
	}

	void push_back(Launcher* launcher)
	{
		m_items.push_back(launcher);
		unset_model();
	}

	void sort();

private:
	void unset_model();

private:
	SectionButton* m_button;
	std::vector<Launcher*> m_items;
	GtkTreeModel* m_model;
};

}

#endif // WHISKERMENU_CATEGORY_HPP
