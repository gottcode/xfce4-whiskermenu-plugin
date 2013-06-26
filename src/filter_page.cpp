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


#include "filter_page.hpp"

#include "launcher_view.hpp"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

FilterPage::FilterPage(Menu* menu) :
	Page(menu),
	m_filter_model(NULL)
{
}

//-----------------------------------------------------------------------------

FilterPage::~FilterPage()
{
	unset_model();
}

//-----------------------------------------------------------------------------

GtkTreeModel* FilterPage::get_model() const
{
	return m_filter_model ? gtk_tree_model_filter_get_model(m_filter_model) : NULL;
}

//-----------------------------------------------------------------------------

void FilterPage::refilter()
{
	if (m_filter_model)
	{
		gtk_tree_model_filter_refilter(m_filter_model);
	}
}

//-----------------------------------------------------------------------------

GtkTreePath* FilterPage::convert_child_path_to_path(GtkTreePath* path) const
{
	return gtk_tree_model_filter_convert_child_path_to_path((m_filter_model), path);
}

//-----------------------------------------------------------------------------

void FilterPage::set_model(GtkTreeModel* model)
{
	unset_model();

	m_filter_model = GTK_TREE_MODEL_FILTER(gtk_tree_model_filter_new(model, NULL));
	gtk_tree_model_filter_set_visible_func(m_filter_model, (GtkTreeModelFilterVisibleFunc)&FilterPage::filter_visible, this, NULL);
	get_view()->set_model(GTK_TREE_MODEL(m_filter_model));
}

//-----------------------------------------------------------------------------

void FilterPage::unset_model()
{
	get_view()->unset_model();

	if (m_filter_model)
	{
		g_object_unref(m_filter_model);
		m_filter_model = NULL;
	}
}

//-----------------------------------------------------------------------------
