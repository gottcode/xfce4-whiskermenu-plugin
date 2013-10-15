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


#include "applications_page.hpp"

#include "category.hpp"
#include "launcher.hpp"
#include "launcher_model.hpp"
#include "launcher_view.hpp"
#include "menu.hpp"
#include "section_button.hpp"

#include <algorithm>

extern "C"
{
#include <libxfce4util/libxfce4util.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static bool f_load_hierarchy = false;

//-----------------------------------------------------------------------------

ApplicationsPage::ApplicationsPage(Menu* menu) :
	Page(menu),
	m_garcon_menu(NULL),
	m_all_button(NULL),
	m_model(NULL),
	m_loaded(false)
{
	// Set desktop environment for applications
	const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
	if (G_LIKELY(!desktop))
	{
		desktop = "XFCE";
	}
	else if (*desktop == '\0')
	{
		desktop = NULL;
	}
	garcon_set_environment(desktop);
}

//-----------------------------------------------------------------------------

ApplicationsPage::~ApplicationsPage()
{
	clear_applications();
}

//-----------------------------------------------------------------------------

Launcher* ApplicationsPage::get_application(const std::string& desktop_id) const
{
	std::map<std::string, Launcher*>::const_iterator i = m_items.find(desktop_id);
	return (i != m_items.end()) ? i->second : NULL;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::apply_filter(GtkToggleButton* togglebutton)
{
	// Only apply filter for active button
	if (gtk_toggle_button_get_active(togglebutton) == false)
	{
		return;
	}

	// Find category matching button
	Category* category = NULL;
	for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		if (GTK_TOGGLE_BUTTON((*i)->get_button()->get_button()) == togglebutton)
		{
			category = *i;
			break;
		}
	}

	// Apply filter
	if (category)
	{
		get_view()->unset_model();
		get_view()->set_fixed_height_mode(!category->has_separators());
		get_view()->set_model(category->get_model());
	}
	else
	{
		get_view()->unset_model();
		get_view()->set_fixed_height_mode(true);
		get_view()->set_model(m_model);
	}
}

//-----------------------------------------------------------------------------

void ApplicationsPage::invalidate_applications()
{
	m_loaded = false;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_applications()
{
	if (m_loaded)
	{
		return;
	}

	// Remove previous menu data
	clear_applications();

	// Populate map of menu data
	m_garcon_menu = garcon_menu_new_applications();
	if (garcon_menu_load(m_garcon_menu, NULL, NULL))
	{
		g_signal_connect_swapped(m_garcon_menu, "reload-required", G_CALLBACK(ApplicationsPage::invalidate_applications_slot), this);
		load_menu(m_garcon_menu, NULL);
	}

	// Sort items and categories
	if (!f_load_hierarchy)
	{
		for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
		{
			(*i)->sort();
		}
		std::sort(m_categories.begin(), m_categories.end(), &Element::less_than);
	}

	// Create sorted list of menu items
	std::vector<Launcher*> sorted_items;
	sorted_items.reserve(m_items.size());
	for (std::map<std::string, Launcher*>::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		sorted_items.push_back(i->second);
	}
	std::sort(sorted_items.begin(), sorted_items.end(), &Element::less_than);

	// Add all items to model
	LauncherModel model;
	for (std::vector<Launcher*>::const_iterator i = sorted_items.begin(), end = sorted_items.end(); i != end; ++i)
	{
		model.append_item(*i);
	}
	m_model = model.get_model();
	g_object_ref(m_model);
	get_view()->set_fixed_height_mode(true);
	get_view()->set_model(m_model);

	// Update filters
	load_categories();

	// Update menu items of other panels
	get_menu()->set_items();

	m_loaded = true;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::clear_applications()
{
	// Free categories
	delete m_all_button;
	m_all_button = NULL;

	for (std::vector<Category*>::iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		delete *i;
	}
	m_categories.clear();

	// Free menu items
	get_menu()->unset_items();
	unset_model();

	for (std::map<std::string, Launcher*>::iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		delete i->second;
	}
	m_items.clear();

	// Unreference menu
	if (m_garcon_menu)
	{
		g_object_unref(m_garcon_menu);
		m_garcon_menu = NULL;
	}

	// Clear menu item cache
	GarconMenuItemCache* cache = garcon_menu_item_cache_get_default();
	garcon_menu_item_cache_invalidate(cache);
	g_object_unref(cache);
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_menu(GarconMenu* menu, Category* parent_category)
{
	GarconMenuDirectory* directory = garcon_menu_get_directory(menu);

	// Skip hidden categories
	if (directory && !garcon_menu_directory_get_visible(directory))
	{
		return;
	}

	// Track categories
	bool first_level = directory && (garcon_menu_get_parent(menu) == m_garcon_menu);
	Category* category = NULL;
	if (directory)
	{
		if (first_level)
		{
			category = new Category(directory);
			m_categories.push_back(category);
		}
		else if (parent_category)
		{
			category = parent_category->append_menu(directory);
		}
	}

	// Add menu elements
	GList* elements = garcon_menu_get_elements(menu);
	for (GList* li = elements; li != NULL; li = li->next)
	{
		if (GARCON_IS_MENU_ITEM(li->data))
		{
			load_menu_item(GARCON_MENU_ITEM(li->data), category);
		}
		else if (GARCON_IS_MENU(li->data))
		{
			load_menu(GARCON_MENU(li->data), category);
		}
		else if (GARCON_IS_MENU_SEPARATOR(li->data) && f_load_hierarchy && category)
		{
			category->append_separator();
		}
	}
	g_list_free(elements);

	// Free unused top-level categories
	if (first_level && category->empty())
	{
		m_categories.erase(std::find(m_categories.begin(), m_categories.end(), category));
		delete category;
		category = NULL;
	}

	// Listen for menu changes
	g_signal_connect_swapped(menu, "directory-changed", G_CALLBACK(ApplicationsPage::invalidate_applications_slot), this);
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_menu_item(GarconMenuItem* menu_item, Category* category)
{
	// Skip hidden items
	if (!garcon_menu_element_get_visible(GARCON_MENU_ELEMENT(menu_item)))
	{
		return;
	}

	// Add to map
	std::string desktop_id(garcon_menu_item_get_desktop_id(menu_item));
	std::map<std::string, Launcher*>::iterator iter = m_items.find(desktop_id);
	if (iter == m_items.end())
	{
		iter = m_items.insert(std::make_pair(desktop_id, new Launcher(menu_item))).first;
	}

	// Add menu item to current category
	if (category)
	{
		category->append_item(iter->second);
	}

	// Listen for menu changes
	g_signal_connect_swapped(menu_item, "changed", G_CALLBACK(ApplicationsPage::invalidate_applications_slot), this);
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_categories()
{
	std::vector<SectionButton*> category_buttons;

	// Add button for all applications
	m_all_button = new SectionButton("applications-other", _("All"));
	g_signal_connect(m_all_button->get_button(), "toggled", G_CALLBACK(ApplicationsPage::apply_filter_slot), this);
	category_buttons.push_back(m_all_button);

	// Add buttons for categories
	for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		SectionButton* category_button = (*i)->get_button();
		g_signal_connect(category_button->get_button(), "toggled", G_CALLBACK(ApplicationsPage::apply_filter_slot), this);
		category_buttons.push_back(category_button);
	}

	// Add category buttons to window
	get_menu()->set_categories(category_buttons);
}

//-----------------------------------------------------------------------------

void ApplicationsPage::unset_model()
{
	if (m_model)
	{
		g_object_unref(m_model);
		m_model = NULL;
	}
}

//-----------------------------------------------------------------------------

bool ApplicationsPage::get_load_hierarchy()
{
	return f_load_hierarchy;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::set_load_hierarchy(bool load)
{
	f_load_hierarchy = load;
}

//-----------------------------------------------------------------------------
