/*
 * Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_APPLICATIONS_PAGE_H
#define WHISKERMENU_APPLICATIONS_PAGE_H

#include "page.h"

#include <map>
#include <string>
#include <vector>

#include <garcon/garcon.h>

namespace WhiskerMenu
{

class Category;
class SectionButton;

class ApplicationsPage : public Page
{

public:
	explicit ApplicationsPage(Window* window);
	~ApplicationsPage();

	Launcher* get_application(const std::string& desktop_id) const;

	void invalidate_applications();
	void load_applications();
	void reload_category_icon_size();

private:
	void apply_filter(GtkToggleButton* togglebutton);
	void clear_applications();
	void load_garcon_menu();
	void load_contents();
	void load_menu(GarconMenu* menu, Category* parent_category);
	void load_menu_item(GarconMenuItem* menu_item, Category* category);

private:
	GarconMenu* m_garcon_menu;
	std::vector<Category*> m_categories;
	std::map<std::string, Launcher*> m_items;
	int m_load_status;
};

}

#endif // WHISKERMENU_APPLICATIONS_PAGE_H
