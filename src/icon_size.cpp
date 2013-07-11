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


#include "icon_size.hpp"

extern "C"
{
#include <glib/gi18n.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

int IconSize::get_size() const
{
	int size = 0;
	switch (m_size)
	{
		case Smallest: size =  16; break;
		case Smaller:  size =  24; break;
		case Small:    size =  32; break;
		case Normal:   size =  48; break;
		case Large:    size =  64; break;
		case Larger:   size =  96; break;
		case Largest:  size = 128; break;
		default:       size =   0; break;
	}
	return size;
}

//-----------------------------------------------------------------------------

std::vector<std::string> IconSize::get_strings()
{
	std::vector<std::string> strings;
	strings.push_back(_("Very Small"));
	strings.push_back(_("Smaller"));
	strings.push_back(_("Small"));
	strings.push_back(_("Normal"));
	strings.push_back(_("Large"));
	strings.push_back(_("Larger"));
	strings.push_back(_("Very Large"));
	return strings;
}

//-----------------------------------------------------------------------------
