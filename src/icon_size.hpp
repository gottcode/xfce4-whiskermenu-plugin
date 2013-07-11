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


#ifndef WHISKERMENU_ICON_SIZE_HPP
#define WHISKERMENU_ICON_SIZE_HPP

#include <string>
#include <vector>

namespace WhiskerMenu
{

class IconSize
{
public:
	enum Size
	{
		Smallest = 0,
		Smaller,
		Small,
		Normal,
		Large,
		Larger,
		Largest
	};

	IconSize(const int size) :
		m_size(size > int(Smallest) ? (size < int(Largest) ? size : int(Largest)) : int(Smallest))
	{
	}

	int get_size() const;

	operator int() const
	{
		return m_size;
	}

	static std::vector<std::string> get_strings();

private:
	int m_size;
};

}

#endif // WHISKERMENU_ICON_SIZE_HPP
