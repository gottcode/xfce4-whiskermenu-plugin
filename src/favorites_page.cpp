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


#include "favorites_page.hpp"

#include "launcher.hpp"
#include "launcher_model.hpp"
#include "launcher_view.hpp"

#include <algorithm>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static const std::string default_favorites[4] =
{
	"exo-terminal-emulator.desktop",
	"exo-file-manager.desktop",
	"exo-mail-reader.desktop",
	"exo-web-browser.desktop"
};

//-----------------------------------------------------------------------------

FavoritesPage::FavoritesPage(XfceRc* settings, Menu* menu) :
	ListPage(settings, "favorites",
	std::vector<std::string>(default_favorites, default_favorites + 4),
	menu)
{
	get_view()->set_reorderable(true);
}

//-----------------------------------------------------------------------------

void FavoritesPage::add(Launcher* launcher)
{
	if (!launcher)
	{
		return;
	}

	// Remove item if already in list
	remove(launcher);

	// Add to list of items
	LauncherModel model(GTK_LIST_STORE(get_view()->get_model()));
	model.append_item(launcher);
}

//-----------------------------------------------------------------------------
