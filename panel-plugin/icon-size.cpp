/*
 * Copyright (C) 2013-2020 Graeme Gott <graeme@gottcode.org>
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

#include "icon-size.h"

#include "settings.h"

#include <glib/gi18n-lib.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

IconSize::IconSize(const gchar* property, const int size) :
	m_property(property),
	m_size(CLAMP(size, NONE, Largest))
{
}

//-----------------------------------------------------------------------------

int IconSize::get_size() const
{
	int size = 0;
	switch (m_size)
	{
		case NONE:     size =   1; break;
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
	return {
		_("None"),
		_("Very Small"),
		_("Smaller"),
		_("Small"),
		_("Normal"),
		_("Large"),
		_("Larger"),
		_("Very Large")
	};
}

//-----------------------------------------------------------------------------

void IconSize::load(XfceRc* rc)
{
	set(xfce_rc_read_int_entry(rc, m_property, m_size));
}

//-----------------------------------------------------------------------------

void IconSize::save(XfceRc* rc)
{
	xfce_rc_write_int_entry(rc, m_property, m_size);
}

//-----------------------------------------------------------------------------

void IconSize::set(int size)
{
	size = CLAMP(size, NONE, Largest);
	if (m_size == size)
	{
		return;
	}

	m_size = size;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------
