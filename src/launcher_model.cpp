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


#include "launcher_model.hpp"

#include "launcher.hpp"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

LauncherModel::LauncherModel()
{
	m_model = gtk_list_store_new(
			N_COLUMNS,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_POINTER);
}

//-----------------------------------------------------------------------------

LauncherModel::LauncherModel(GtkListStore* model) :
	m_model(model)
{
	if (G_LIKELY(m_model))
	{
		g_object_ref(m_model);
	}
}

//-----------------------------------------------------------------------------

LauncherModel::~LauncherModel()
{
	if (G_LIKELY(m_model))
	{
		g_object_unref(m_model);
	}
}

//-----------------------------------------------------------------------------

void LauncherModel::insert_item(Launcher* launcher, int position)
{
	gtk_list_store_insert_with_values(
			m_model, NULL, position,
			COLUMN_ICON, launcher->get_icon(),
			COLUMN_TEXT, launcher->get_text(),
			COLUMN_LAUNCHER, launcher,
			-1);
}

//-----------------------------------------------------------------------------

void LauncherModel::remove_item(Launcher* launcher)
{
	GtkTreeModel* model = GTK_TREE_MODEL(m_model);
	GtkTreeIter iter;
	Launcher* test_launcher = NULL;

	bool valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid)
	{
		gtk_tree_model_get(model, &iter, COLUMN_LAUNCHER, &test_launcher, -1);
		if (test_launcher == launcher)
		{
			gtk_list_store_remove(m_model, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(model, &iter);
	}
}

//-----------------------------------------------------------------------------

void LauncherModel::remove_first_item()
{
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_model), &iter))
	{
		gtk_list_store_remove(m_model, &iter);
	}
}

//-----------------------------------------------------------------------------

void LauncherModel::remove_last_item()
{
	GtkTreeModel* model = GTK_TREE_MODEL(m_model);
	gint size = gtk_tree_model_iter_n_children(model, NULL);
	if (!size)
	{
		return;
	}

	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(model, &iter, NULL, size - 1))
	{
		gtk_list_store_remove(m_model, &iter);
	}
}

//-----------------------------------------------------------------------------
