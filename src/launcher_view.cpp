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


#include "launcher_view.hpp"

#include "launcher_model.hpp"

#include <algorithm>

extern "C"
{
#include <exo/exo.h>
#include <gdk/gdkkeysyms.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

LauncherView::LauncherView() :
	m_model(nullptr)
{
	// Create the view
	m_view = GTK_TREE_VIEW(exo_tree_view_new());
	gtk_tree_view_set_headers_visible(m_view, false);
	gtk_tree_view_set_enable_tree_lines(m_view, false);
	gtk_tree_view_set_rules_hint(m_view, false);
	gtk_tree_view_set_hover_selection(m_view, true);
	gtk_tree_view_set_enable_search(m_view, false);
	g_signal_connect(m_view, "key-press-event", SLOT_CALLBACK(LauncherView::on_key_press_event), this);
	g_signal_connect(m_view, "key-release-event", SLOT_CALLBACK(LauncherView::on_key_release_event), this);

	// Add a column for the icon and text
	GtkTreeViewColumn* column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_expand(column, true);
	gtk_tree_view_column_set_visible(column, true);

	int width = 0, height = 0;
	gtk_icon_size_lookup(GTK_ICON_SIZE_DND, &width, &height);
	GtkCellRenderer* icon_renderer = exo_cell_renderer_icon_new();
	g_object_set(icon_renderer, "size", std::max(width, height), nullptr);
	gtk_tree_view_column_pack_start(column, icon_renderer, false);
	gtk_tree_view_column_add_attribute(column, icon_renderer, "icon", LauncherModel::COLUMN_ICON);

	GtkCellRenderer* text_renderer = gtk_cell_renderer_text_new();
	g_object_set(text_renderer, "ellipsize", PANGO_ELLIPSIZE_END, nullptr);
	gtk_tree_view_column_pack_start(column, text_renderer, true);
	gtk_tree_view_column_add_attribute(column, text_renderer, "markup", LauncherModel::COLUMN_TEXT);

	gtk_tree_view_append_column(m_view, column);

	// Use single clicks to activate items
	exo_tree_view_set_single_click(EXO_TREE_VIEW(m_view), true);

	// Only allow up to one selected item
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	g_object_ref_sink(m_view);
}

//-----------------------------------------------------------------------------

LauncherView::~LauncherView()
{
	m_model = nullptr;

	g_object_unref(m_view);
}

//-----------------------------------------------------------------------------

GtkTreePath* LauncherView::get_selected_path() const
{
	GtkTreePath* path = nullptr;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(selection, nullptr, &iter))
	{
		path = gtk_tree_model_get_path(m_model, &iter);
	}
	return path;
}

//-----------------------------------------------------------------------------

void LauncherView::activate_path(GtkTreePath* path)
{
	GtkTreeViewColumn* column = gtk_tree_view_get_column(m_view, 0);
	gtk_tree_view_row_activated(m_view, path, column);
}

//-----------------------------------------------------------------------------

void LauncherView::scroll_to_path(GtkTreePath* path)
{
	gtk_tree_view_scroll_to_cell(m_view, path, nullptr, true, 0.5f, 0.5f);
}

//-----------------------------------------------------------------------------

void LauncherView::select_path(GtkTreePath* path)
{
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	gtk_tree_selection_select_path(selection, path);
}

//-----------------------------------------------------------------------------

void LauncherView::set_reorderable(bool reorderable)
{
	gtk_tree_view_set_reorderable(m_view, reorderable);
}

//-----------------------------------------------------------------------------

void LauncherView::set_selection_mode(GtkSelectionMode mode)
{
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	gtk_tree_selection_set_mode(selection, mode);
}

//-----------------------------------------------------------------------------

void LauncherView::unselect_all()
{
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	gtk_tree_selection_unselect_all(selection);
}

//-----------------------------------------------------------------------------

void LauncherView::set_model(GtkTreeModel* model)
{
	m_model = model;
	gtk_tree_view_set_model(m_view, model);
}

//-----------------------------------------------------------------------------

void LauncherView::unset_model()
{
	m_model = nullptr;
	gtk_tree_view_set_model(m_view, nullptr);
}

//-----------------------------------------------------------------------------

gboolean LauncherView::on_key_press_event(GtkWidget*, GdkEventKey* event)
{
	if ((event->keyval == GDK_KEY_Up) || (event->keyval == GDK_KEY_Down))
	{
		gtk_tree_view_set_hover_selection(m_view, false);
	}
	return false;
}

//-----------------------------------------------------------------------------

gboolean LauncherView::on_key_release_event(GtkWidget*, GdkEventKey* event)
{
	if ((event->keyval == GDK_KEY_Up) || (event->keyval == GDK_KEY_Down))
	{
		gtk_tree_view_set_hover_selection(m_view, true);
	}
	return false;
}

//-----------------------------------------------------------------------------
