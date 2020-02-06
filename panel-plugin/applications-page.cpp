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

#include "applications-page.h"

#include "category.h"
#include "launcher.h"
#include "launcher-view.h"
#include "section-button.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

#include <algorithm>

extern "C"
{
#include <libxfce4util/libxfce4util.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

ApplicationsPage::ApplicationsPage(Window* window) :
	Page(window),
	m_garcon_menu(nullptr),
	m_garcon_settings_menu(nullptr),
	m_status(LoadStatus::Invalid)
{
	// Set desktop environment for applications
	const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
	if (G_LIKELY(!desktop))
	{
		desktop = "XFCE";
	}
	else if (*desktop == '\0')
	{
		desktop = nullptr;
	}
	garcon_set_environment(desktop);
}

//-----------------------------------------------------------------------------

ApplicationsPage::~ApplicationsPage()
{
	clear();
}

//-----------------------------------------------------------------------------

GtkTreeModel* ApplicationsPage::create_launcher_model(std::vector<std::string>& desktop_ids) const
{
	// Create new model for treeview
	GtkListStore* store = gtk_list_store_new(
			LauncherView::N_COLUMNS,
			G_TYPE_ICON,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_POINTER);

	// Fetch menu items or remove them from list if missing
	for (auto i = desktop_ids.begin(); i != desktop_ids.end(); ++i)
	{
		if (i->empty())
		{
			continue;
		}

		Launcher* launcher = find(*i);
		if (launcher)
		{
			gtk_list_store_insert_with_values(
					store, nullptr, G_MAXINT,
					LauncherView::COLUMN_ICON, launcher->get_icon(),
					LauncherView::COLUMN_TEXT, launcher->get_text(),
					LauncherView::COLUMN_TOOLTIP, launcher->get_tooltip(),
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

Launcher* ApplicationsPage::find(const std::string& desktop_id) const
{
	auto i = m_items.find(desktop_id);
	return (i != m_items.end()) ? i->second : nullptr;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::apply_filter(GtkToggleButton* togglebutton)
{
	// Only apply filter for active button
	if (!gtk_toggle_button_get_active(togglebutton))
	{
		return;
	}

	// Find category matching button
	Category* category = nullptr;
	for (auto test_category : m_categories)
	{
		if (GTK_TOGGLE_BUTTON(test_category->get_button()->get_widget()) == togglebutton)
		{
			category = test_category;
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

void ApplicationsPage::invalidate()
{
	if (m_status == LoadStatus::Done)
	{
		m_status = LoadStatus::Invalid;
	}
	else if (m_status == LoadStatus::Loading)
	{
		m_status = LoadStatus::ReloadRequired;
	}
}

//-----------------------------------------------------------------------------

bool ApplicationsPage::load()
{
	// Check if already loaded
	if (m_status == LoadStatus::Done)
	{
		return true;
	}
	// Check if currently loading
	else if (m_status != LoadStatus::Invalid)
	{
		return false;
	}
	m_status = LoadStatus::Loading;

	// Load menu
	clear();

	// Load contents in thread if possible
	GTask* task = g_task_new(nullptr, nullptr, &ApplicationsPage::load_contents_slot, this);
	g_task_set_task_data(task, this, nullptr);
	g_task_run_in_thread(task, &ApplicationsPage::load_garcon_menu_slot);
	g_object_unref(task);

	return false;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::reload_category_icon_size()
{
	for (auto category : m_categories)
	{
		category->get_button()->reload_icon_size();
	}
}

//-----------------------------------------------------------------------------

void ApplicationsPage::clear()
{
	// Free categories
	for (auto category : m_categories)
	{
		delete category;
	}
	m_categories.clear();

	// Free menu items
	get_window()->unset_items();
	get_view()->unset_model();

	for (const auto& i : m_items)
	{
		delete i.second;
	}
	m_items.clear();

	// Free menu
	if (G_LIKELY(m_garcon_menu))
	{
		g_object_unref(m_garcon_menu);
		m_garcon_menu = nullptr;
	}

	// Free settings menu
	if (G_LIKELY(m_garcon_settings_menu))
	{
		g_object_unref(m_garcon_settings_menu);
		m_garcon_settings_menu = nullptr;
	}
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_garcon_menu()
{
	// Create menu
	if (wm_settings->custom_menu_file.empty())
	{
		m_garcon_menu = garcon_menu_new_applications();
	}
	else
	{
		m_garcon_menu = garcon_menu_new_for_path(wm_settings->custom_menu_file.c_str());
	}

	// Load menu
	if (m_garcon_menu && !garcon_menu_load(m_garcon_menu, nullptr, nullptr))
	{
		g_object_unref(m_garcon_menu);
		m_garcon_menu = nullptr;
	}

	if (!m_garcon_menu)
	{
		return;
	}

	g_signal_connect_slot<GarconMenu*>(m_garcon_menu, "reload-required", &ApplicationsPage::invalidate, this);
	load_menu(m_garcon_menu, nullptr);

	// Create settings menu
	gchar* path = xfce_resource_lookup(XFCE_RESOURCE_CONFIG, "menus/xfce-settings-manager.menu");
	m_garcon_settings_menu = garcon_menu_new_for_path(path ? path : SETTINGS_MENUFILE);
	g_free(path);

	if (m_garcon_settings_menu)
	{
		g_signal_connect_slot<GarconMenu*>(m_garcon_settings_menu, "reload-required", &ApplicationsPage::invalidate, this);
	}

	// Load settings menu
	if (m_garcon_settings_menu && garcon_menu_load(m_garcon_settings_menu, nullptr, nullptr))
	{
		load_menu(m_garcon_settings_menu, nullptr);
	}

	// Sort items and categories
	if (!wm_settings->load_hierarchy)
	{
		for (auto category : m_categories)
		{
			category->sort();
		}
		std::sort(m_categories.begin(), m_categories.end(), &Element::less_than);
	}

	// Create all items category
	Category* category = new Category(nullptr);
	for (const auto& i : m_items)
	{
		category->append_item(i.second);
	}
	category->sort();
	m_categories.insert(m_categories.begin(), category);
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_contents()
{
	if (!m_garcon_menu)
	{
		get_window()->set_loaded();

		m_status = LoadStatus::Invalid;

		return;
	}

	// Set all applications category
	get_view()->set_fixed_height_mode(true);
	get_view()->set_model(m_categories.front()->get_model());

	// Add buttons for categories
	std::vector<SectionButton*> category_buttons;
	for (auto category : m_categories)
	{
		SectionButton* category_button = category->get_button();
		g_signal_connect_slot(category_button->get_widget(), "toggled", &ApplicationsPage::apply_filter, this);
		category_buttons.push_back(category_button);
	}

	// Add category buttons to window
	get_window()->set_categories(category_buttons);

	// Update menu items of other panels
	get_window()->set_items();
	get_window()->set_loaded();

	m_status = (m_status == LoadStatus::Loading) ? LoadStatus::Done : LoadStatus::Invalid;
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
	Category* category = nullptr;
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
	for (GList* li = elements; li; li = li->next)
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
		category = nullptr;
	}

	// Listen for menu changes
	g_signal_connect_slot<GarconMenu*,GarconMenuDirectory*,GarconMenuDirectory*>(menu, "directory-changed", &ApplicationsPage::invalidate, this);
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
	auto iter = m_items.find(desktop_id);
	if (iter == m_items.end())
	{
		iter = m_items.insert(std::make_pair(std::move(desktop_id), new Launcher(menu_item))).first;
	}

	// Add menu item to current category
	if (category)
	{
		category->append_item(iter->second);
	}

	// Listen for menu changes
	g_signal_connect_slot<GarconMenuItem*>(menu_item, "changed", &ApplicationsPage::invalidate, this);
}

//-----------------------------------------------------------------------------
