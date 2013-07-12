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

#include <map>
#include <string>
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
class SectionButton;

class ApplicationsPage : public FilterPage
{
	class Category
	{
	public:
		explicit Category(GarconMenuDirectory* directory);

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
	explicit ApplicationsPage(Menu* menu);
	~ApplicationsPage();

	Launcher* get_application(const std::string& desktop_id) const;

	void load_applications();

public:
	SLOT_0(void, ApplicationsPage, invalidate_applications);

private:
	SLOT_1(void, ApplicationsPage, apply_filter, GtkToggleButton*);

private:
	bool on_filter(GtkTreeModel* model, GtkTreeIter* iter);
	void clear_applications();
	void load_menu(GarconMenu* menu);
	static void load_menu_item(const gchar* desktop_id, GarconMenuItem* menu_item, ApplicationsPage* page);
	void load_categories();

private:
	GarconMenu* m_garcon_menu;
	Category* m_current_category;
	std::map<SectionButton*, Category*> m_category_buttons;
	std::map<Category*, std::vector<Launcher*> > m_categories;
	std::map<std::string, Launcher*> m_items;
	bool m_loaded;
};

}

#endif // WHISKERMENU_APPLICATIONS_PAGE_HPP
