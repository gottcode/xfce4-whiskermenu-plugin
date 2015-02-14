/*
 * Copyright (C) 2013, 2015 Graeme Gott <graeme@gottcode.org>
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

#include "applications-page.h"

#include "category.h"
#include "launcher.h"
#include "launcher-view.h"
#include "section-button.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

#include <algorithm>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

enum
{
	STATUS_INVALID,
	STATUS_LOADING,
	STATUS_LOADED
};

//-----------------------------------------------------------------------------

ApplicationsPage::ApplicationsPage(Window* window) :
	Page(window),
	m_garcon_menu(NULL),
	m_load_status(STATUS_INVALID)
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

GtkTreeModel* ApplicationsPage::create_launcher_model(std::vector<std::string>& desktop_ids) const
{
	// Create new model for treeview
	GtkListStore* store = gtk_list_store_new(
			LauncherView::N_COLUMNS,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_POINTER);

	// Fetch menu items or remove them from list if missing
	for (std::vector<std::string>::iterator i = desktop_ids.begin(); i != desktop_ids.end(); ++i)
	{
		if (i->empty())
		{
			continue;
		}

		Launcher* launcher = get_application(*i);
		if (launcher)
		{
			gtk_list_store_insert_with_values(
					store, NULL, G_MAXINT,
					LauncherView::COLUMN_ICON, launcher->get_icon(),
					LauncherView::COLUMN_TEXT, launcher->get_text(),
					LauncherView::COLUMN_LAUNCHER, launcher,
					-1);
		}
		else
		{
			i = desktop_ids.erase(i);
			--i;
			wm_settings->set_modified();
		}
	}

	return GTK_TREE_MODEL(store);
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
	if (!category)
	{
		return;
	}

	// Apply filter
	get_view()->unset_model();
	get_view()->set_fixed_height_mode(!category->has_separators());
	get_view()->set_model(category->get_model());
}

//-----------------------------------------------------------------------------

void ApplicationsPage::invalidate_applications()
{
	m_load_status = STATUS_INVALID;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_applications()
{
	// Check if already loaded
	if (m_load_status == STATUS_LOADED)
	{
		return;
	}
	m_load_status = STATUS_LOADING;

	// Load menu
	clear_applications();
	load_contents();

	return;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::reload_category_icon_size()
{
	for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		(*i)->get_button()->reload_icon_size();
	}
}

//-----------------------------------------------------------------------------

void ApplicationsPage::clear_applications()
{
	// Free categories
	for (std::vector<Category*>::iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		delete *i;
	}
	m_categories.clear();

	// Free menu items
	get_window()->unset_items();
	get_view()->unset_model();

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
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_contents()
{
	// Load garcon menu
	if (wm_settings->custom_menu_file.empty())
	{
		m_garcon_menu = garcon_menu_new_applications();
	}
	else
	{
		m_garcon_menu = garcon_menu_new_for_path(wm_settings->custom_menu_file.c_str());
	}

	if (m_garcon_menu && !garcon_menu_load(m_garcon_menu, NULL, NULL))
	{
		g_object_unref(m_garcon_menu);
		m_garcon_menu = NULL;
	}

	if (!m_garcon_menu)
	{
		m_load_status = STATUS_INVALID;

		return;
	}

	// Populate map of menu data
	g_signal_connect_slot<GarconMenu*>(m_garcon_menu, "reload-required", &ApplicationsPage::invalidate_applications, this);
	load_menu(m_garcon_menu, NULL);

	// Sort items and categories
	if (!wm_settings->load_hierarchy)
	{
		for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
		{
			(*i)->sort();
		}
		std::sort(m_categories.begin(), m_categories.end(), &Element::less_than);
	}

	// Create all items category
	Category* category = new Category(NULL);
	for (std::map<std::string, Launcher*>::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		category->append_item(i->second);
	}
	category->sort();
	m_categories.insert(m_categories.begin(), category);

	// Set all applications category
	get_view()->set_fixed_height_mode(true);
	get_view()->set_model(category->get_model());

	// Add buttons for categories
	std::vector<SectionButton*> category_buttons;
	for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		SectionButton* category_button = (*i)->get_button();
		g_signal_connect_slot(category_button->get_button(), "toggled", &ApplicationsPage::apply_filter, this);
		category_buttons.push_back(category_button);
	}

	// Add category buttons to window
	get_window()->set_categories(category_buttons);

	// Update menu items of other panels
	get_window()->set_items();

	m_load_status = STATUS_LOADED;
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
		else if (!wm_settings->load_hierarchy)
		{
			category = parent_category;
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
		else if (GARCON_IS_MENU_SEPARATOR(li->data) && wm_settings->load_hierarchy && category)
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
	g_signal_connect_slot<GarconMenu*,GarconMenuDirectory*,GarconMenuDirectory*>(menu, "directory-changed", &ApplicationsPage::invalidate_applications, this);
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
	g_signal_connect_slot<GarconMenuItem*>(menu_item, "changed", &ApplicationsPage::invalidate_applications, this);
}

//-----------------------------------------------------------------------------
