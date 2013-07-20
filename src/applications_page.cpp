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

extern "C"
{
#include <libxfce4util/libxfce4util.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

ApplicationsPage::ApplicationsPage(Menu* menu) :
	FilterPage(menu),
	m_garcon_menu(NULL),
	m_current_category(NULL),
	m_all_button(NULL),
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
	m_current_category = NULL;
	for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		if (GTK_TOGGLE_BUTTON((*i)->get_button()->get_button()) == togglebutton)
		{
			m_current_category = *i;
			break;
		}
	}

	// Apply filter
	refilter();
	m_current_category = NULL;
}

//-----------------------------------------------------------------------------

bool ApplicationsPage::on_filter(GtkTreeModel* model, GtkTreeIter* iter)
{
	if (!m_current_category)
	{
		return true;
	}

	Launcher* launcher = NULL;
	gtk_tree_model_get(model, iter, LauncherModel::COLUMN_LAUNCHER, &launcher, -1);
	if (!launcher)
	{
		return false;
	}

	return m_current_category->contains(launcher);
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
	g_object_ref(m_garcon_menu);
	if (garcon_menu_load(m_garcon_menu, NULL, NULL))
	{
		g_signal_connect_swapped(m_garcon_menu, "reload-required", G_CALLBACK(ApplicationsPage::invalidate_applications_slot), this);
		load_menu(m_garcon_menu);
	}

	// Create sorted list of menu items
	std::multimap<std::string, Launcher*> sorted_items;
	for (std::map<std::string, Launcher*>::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		gchar* collation_key = g_utf8_collate_key(i->second->get_text(), -1);
		sorted_items.insert(std::make_pair(collation_key, i->second));
		g_free(collation_key);
	}

	// Add all items to model
	LauncherModel model;
	for (std::multimap<std::string, Launcher*>::const_iterator i = sorted_items.begin(), end = sorted_items.end(); i != end; ++i)
	{
		model.append_item(i->second);
	}

	// Create filter model and pass ownership to view, do not delete!
	set_model(model.get_model());

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

void ApplicationsPage::load_menu(GarconMenu* menu)
{
	GarconMenuDirectory* directory = garcon_menu_get_directory(menu);

	// Skip hidden categories
	if (directory && !garcon_menu_directory_get_visible(directory))
	{
		g_object_unref(directory);
		return;
	}

	// Only track single level of categories
	bool first_level = directory && (garcon_menu_get_parent(menu) == m_garcon_menu);
	if (first_level)
	{
		m_current_category = new Category(directory);
		m_categories.push_back(m_current_category);
	}
	if (directory)
	{
		g_object_unref(directory);
	}

	// Add submenus
	GList* menus = garcon_menu_get_menus(menu);
	for (GList* li = menus; li != NULL; li = li->next)
	{
		load_menu(GARCON_MENU(li->data));
	}
	g_list_free(menus);

	// Add items
	GarconMenuItemPool* pool = garcon_menu_get_item_pool(menu);
	if (G_LIKELY(pool))
	{
		garcon_menu_item_pool_foreach(pool, (GHFunc)&ApplicationsPage::load_menu_item, this);
	}

	// Only track single level of categories
	if (first_level)
	{
		// Free unused categories
		if (m_current_category->empty())
		{
			m_categories.erase(std::find(m_categories.begin(), m_categories.end(), m_current_category));
			delete m_current_category;
		}
		m_current_category = NULL;
	}

	// Listen for menu changes
	g_signal_connect_swapped(menu, "directory-changed", G_CALLBACK(ApplicationsPage::invalidate_applications_slot), this);
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_menu_item(const gchar* desktop_id, GarconMenuItem* menu_item, ApplicationsPage* page)
{
	// Skip hidden items
	if (!garcon_menu_element_get_visible(GARCON_MENU_ELEMENT(menu_item)))
	{
		return;
	}

	// Add to map
	std::string key(desktop_id);
	std::map<std::string, Launcher*>::iterator iter = page->m_items.find(key);
	if (iter == page->m_items.end())
	{
		iter = page->m_items.insert(std::make_pair(key, new Launcher(menu_item))).first;
	}

	// Add menu item to current category
	if (page->m_current_category)
	{
		page->m_current_category->push_back(iter->second);
	}

	// Listen for menu changes
	g_signal_connect_swapped(menu_item, "changed", G_CALLBACK(ApplicationsPage::invalidate_applications_slot), page);
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
