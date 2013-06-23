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


#ifndef WHISKERMENU_APPLICATIONS_PAGE_HPP
#define WHISKERMENU_APPLICATIONS_PAGE_HPP

#include "filter_page.hpp"
#include "slot.hpp"

#include <string>
#include <unordered_map>
#include <vector>

extern "C"
{
#include <garcon/garcon.h>
}

namespace WhiskerMenu
{

class Launcher;
class LauncherView;
class Menu;

class ApplicationsPage : public FilterPage
{
	class Category
	{
	public:
		Category(GarconMenuDirectory* directory);

		const gchar* get_icon() const
		{
			return m_icon.c_str();
		}

		const gchar* get_text() const
		{
			return m_text.c_str();
		}

	private:
		std::string m_icon;
		std::string m_text;
	};

public:
	ApplicationsPage(Menu* menu);
	~ApplicationsPage();

	SLOT_0(void, ApplicationsPage, reload_applications);

private:
	SLOT_1(void, ApplicationsPage, apply_filter, GtkToggleButton*);

private:
	bool on_filter(GtkTreeModel* model, GtkTreeIter* iter);
	void clear_applications();
	void load_menu(GarconMenu* menu);
	static void load_menu_item(const gchar* desktop_id, GarconMenuItem* menu_item, ApplicationsPage* page);
	void reload_categories();

private:
	GarconMenu* m_garcon_menu;
	Category* m_current_category;
	std::unordered_map<GtkRadioButton*, Category*> m_category_buttons;
	std::unordered_map<Category*, std::vector<Launcher*>> m_categories;
	std::unordered_map<std::string, Launcher*> m_items;
};

}

#endif // WHISKERMENU_APPLICATIONS_PAGE_HPP
