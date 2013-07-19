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


#include "category.hpp"

#include "section_button.hpp"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

Category::Category(GarconMenuDirectory* directory)
{
	// Fetch icon
	const gchar* icon = garcon_menu_directory_get_icon_name(directory);
	if (G_UNLIKELY(!icon))
	{
		icon = "";
	}

	// Fetch text
	const gchar* text = garcon_menu_directory_get_name(directory);
	if (G_UNLIKELY(!text))
	{
		text = "";
	}

	// Create button
	m_button = new SectionButton(icon, text);
}

//-----------------------------------------------------------------------------

Category::~Category()
{
	delete m_button;
}

//-----------------------------------------------------------------------------
