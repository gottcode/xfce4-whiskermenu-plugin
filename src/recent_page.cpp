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


#include "recent_page.hpp"

#include "launcher.hpp"
#include "launcher_model.hpp"
#include "launcher_view.hpp"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

RecentPage::RecentPage(XfceRc* settings, Menu* menu) :
	ListPage(settings, "recent", std::vector<std::string>(), menu),
	m_max_items(10)
{
	// Prevent going over max
	LauncherModel model(GTK_LIST_STORE(get_view()->get_model()));
	while (size() > m_max_items)
	{
		model.remove_last_item();
	}
}

//-----------------------------------------------------------------------------

void RecentPage::add(Launcher* launcher)
{
	// Remove item if already in list
	remove(launcher);

	// Prepend to list of items
	LauncherModel model(GTK_LIST_STORE(get_view()->get_model()));
	model.prepend_item(launcher);

	// Prevent going over max
	while (size() > m_max_items)
	{
		model.remove_last_item();
	}
}

//-----------------------------------------------------------------------------
