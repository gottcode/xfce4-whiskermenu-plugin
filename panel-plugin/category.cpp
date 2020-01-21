/*
 * Copyright (C) 2013, 2016, 2020 Graeme Gott <graeme@gottcode.org>
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

#include "category.h"

#include "launcher-view.h"
#include "section-button.h"

#include <algorithm>

#include <exo/exo.h>
#include <glib/gi18n-lib.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static bool is_null(const Element* element)
{
	return !element;
}

//-----------------------------------------------------------------------------

Category::Category(GarconMenuDirectory* directory) :
	m_button(NULL),
	m_model(NULL),
	m_has_separators(false),
	m_has_subcategories(false)
{
	const gchar* icon = NULL;
	const gchar* text = NULL;
	const gchar* tooltip = NULL;
	if (directory)
	{
		icon = garcon_menu_directory_get_icon_name(directory);
		text = garcon_menu_directory_get_name(directory);
		tooltip = garcon_menu_directory_get_comment(directory);
	}
	else
	{
		text = _("All Applications");
	}
	set_icon(!exo_str_is_empty(icon) ? icon : "applications-other");
	set_text(text ? text : "");
	set_tooltip(tooltip ? tooltip : "");
}

//-----------------------------------------------------------------------------

Category::~Category()
{
	unset_model();

	delete m_button;

	for (std::vector<Element*>::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		if (Category* category = dynamic_cast<Category*>(*i))
		{
			delete category;
		}
	}
}

//-----------------------------------------------------------------------------

SectionButton* Category::get_button()
{
	if (!m_button)
	{
		m_button = new SectionButton(get_icon(), get_text());
	}

	return m_button;
}

//-----------------------------------------------------------------------------

GtkTreeModel* Category::get_model()
{
	if (!m_model)
	{
		if (m_has_subcategories)
		{
			GtkTreeStore* model = gtk_tree_store_new(
					LauncherView::N_COLUMNS,
					G_TYPE_ICON,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_POINTER);
			insert_items(model, NULL);
			m_model = GTK_TREE_MODEL(model);
		}
		else
		{
			GtkListStore* model = gtk_list_store_new(
					LauncherView::N_COLUMNS,
					G_TYPE_ICON,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_POINTER);
			insert_items(model);
			m_model = GTK_TREE_MODEL(model);
		}
	}

	return m_model;
}

//-----------------------------------------------------------------------------

bool Category::empty() const
{
	for (std::vector<Element*>::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		Category* category = dynamic_cast<Category*>(*i);
		if ((!category && *i) || !category->empty())
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

Category* Category::append_menu(GarconMenuDirectory* directory)
{
	m_has_subcategories = true;
	unset_model();
	Category* category = new Category(directory);
	m_items.push_back(category);
	return category;
}

//-----------------------------------------------------------------------------

void Category::append_separator()
{
	if (!m_items.empty() && m_items.back())
	{
		unset_model();
		m_items.push_back(NULL);
		m_has_separators = true;
	}
}

//-----------------------------------------------------------------------------

void Category::sort()
{
	unset_model();
	merge();
	if (m_has_separators)
	{
		m_items.erase(std::remove_if(m_items.begin(), m_items.end(), is_null), m_items.end());
	}
	std::sort(m_items.begin(), m_items.end(), &Element::less_than);
}

//-----------------------------------------------------------------------------

void Category::insert_items(GtkTreeStore* model, GtkTreeIter* parent)
{
	for (std::vector<Element*>::size_type i = 0, end = m_items.size(); i < end; ++i)
	{
		Element* element = m_items[i];
		if (Category* category = dynamic_cast<Category*>(element))
		{
			if (category->empty())
			{
				continue;
			}

			gchar* text = g_markup_escape_text(category->get_text(), -1);
			const gchar* tooltip = category->get_tooltip();

			GtkTreeIter iter;
			gtk_tree_store_insert_with_values(model,
					&iter, parent, INT_MAX,
					LauncherView::COLUMN_ICON, category->get_icon(),
					LauncherView::COLUMN_TEXT, text,
					LauncherView::COLUMN_TOOLTIP, tooltip,
					LauncherView::COLUMN_LAUNCHER, NULL,
					-1);
			g_free(text);
			category->insert_items(model, &iter);
		}
		else if (Launcher* launcher = dynamic_cast<Launcher*>(element))
		{
			gtk_tree_store_insert_with_values(model,
					NULL, parent, INT_MAX,
					LauncherView::COLUMN_ICON, launcher->get_icon(),
					LauncherView::COLUMN_TEXT, launcher->get_text(),
					LauncherView::COLUMN_TOOLTIP, launcher->get_tooltip(),
					LauncherView::COLUMN_LAUNCHER, launcher,
					-1);
		}
		else if ((i + 1) < end)
		{
			gtk_tree_store_insert_with_values(model,
					NULL, parent, INT_MAX,
					LauncherView::COLUMN_ICON, NULL,
					LauncherView::COLUMN_TEXT, NULL,
					LauncherView::COLUMN_TOOLTIP, NULL,
					LauncherView::COLUMN_LAUNCHER, NULL,
					-1);
		}
	}
}

//-----------------------------------------------------------------------------

void Category::insert_items(GtkListStore* model)
{
	for (std::vector<Element*>::size_type i = 0, end = m_items.size(); i < end; ++i)
	{
		Element* element = m_items[i];
		if (Launcher* launcher = dynamic_cast<Launcher*>(element))
		{
			gtk_list_store_insert_with_values(model,
					NULL, INT_MAX,
					LauncherView::COLUMN_ICON, launcher->get_icon(),
					LauncherView::COLUMN_TEXT, launcher->get_text(),
					LauncherView::COLUMN_TOOLTIP, launcher->get_tooltip(),
					LauncherView::COLUMN_LAUNCHER, launcher,
					-1);
		}
		else if ((i + 1) < end)
		{
			gtk_list_store_insert_with_values(model,
					NULL, INT_MAX,
					LauncherView::COLUMN_ICON, NULL,
					LauncherView::COLUMN_TEXT, NULL,
					LauncherView::COLUMN_TOOLTIP, NULL,
					LauncherView::COLUMN_LAUNCHER, NULL,
					-1);
		}
	}
}

//-----------------------------------------------------------------------------

void Category::merge()
{
	if (!m_has_subcategories)
	{
		return;
	}

	// Find direct subcategories
	std::vector<Category*> categories;
	for (std::vector<Element*>::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		if (Category* category = dynamic_cast<Category*>(*i))
		{
			categories.push_back(category);
		}
	}
	std::vector<Category*>::size_type last_direct = categories.size();

	// Recursively find subcategories
	std::vector<Element*>::size_type count = m_items.size();
	for (std::vector<Category*>::size_type i = 0; i < categories.size(); ++i)
	{
		Category* category = categories[i];
		count += category->m_items.size();

		for (std::vector<Element*>::const_iterator j = category->m_items.begin(), end = category->m_items.end(); j != end; ++j)
		{
			if (Category* subcategory = dynamic_cast<Category*>(*j))
			{
				categories.push_back(subcategory);
			}
		}
	}

	// Append items
	m_items.reserve(count);
	for (std::vector<Category*>::const_iterator i = categories.begin(), end = categories.end(); i != end; ++i)
	{
		m_items.insert(m_items.end(), (*i)->m_items.begin(), (*i)->m_items.end());
	}

	// Remove subcategories
	for (std::vector<Element*>::iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		if (dynamic_cast<Category*>(*i))
		{
			*i = NULL;
		}
	}

	// Delete direct subcategories; they will recursively delete their subcategories
	for (std::vector<Category*>::size_type i = 0; i < last_direct; ++i)
	{
		delete categories[i];
	}

	m_has_subcategories = false;
	m_has_separators = true;
}

//-----------------------------------------------------------------------------

void Category::unset_model()
{
	if (m_model)
	{
		g_object_unref(m_model);
		m_model = NULL;
	}
}

//-----------------------------------------------------------------------------
