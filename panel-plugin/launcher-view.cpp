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

#include "launcher-view.h"

#include "settings.h"
#include "slot.h"

#include <algorithm>

#include <exo/exo.h>
#include <gdk/gdkkeysyms.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static gboolean is_separator(GtkTreeModel* model, GtkTreeIter* iter, gpointer)
{
	const gchar* text;
	gtk_tree_model_get(model, iter, LauncherView::COLUMN_TEXT, &text, -1);
	return exo_str_is_empty(text);
}

//-----------------------------------------------------------------------------

LauncherView::LauncherView() :
	m_model(NULL),
	m_icon_size(0)
{
	// Create the view
	m_view = GTK_TREE_VIEW(exo_tree_view_new());
	gtk_tree_view_set_headers_visible(m_view, false);
	gtk_tree_view_set_enable_tree_lines(m_view, false);
	gtk_tree_view_set_rules_hint(m_view, false);
	gtk_tree_view_set_hover_selection(m_view, true);
	gtk_tree_view_set_enable_search(m_view, false);
	gtk_tree_view_set_fixed_height_mode(m_view, true);
	gtk_tree_view_set_row_separator_func(m_view, &is_separator, NULL, NULL);
	create_column();
	g_signal_connect_slot(m_view, "key-press-event", &LauncherView::on_key_press_event, this);
	g_signal_connect_slot(m_view, "key-release-event", &LauncherView::on_key_release_event, this);

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
	m_model = NULL;

	g_object_unref(m_view);
}

//-----------------------------------------------------------------------------

GtkTreePath* LauncherView::get_selected_path() const
{
	GtkTreePath* path = NULL;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
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
	gtk_tree_view_scroll_to_cell(m_view, path, NULL, true, 0.5f, 0.5f);
}

//-----------------------------------------------------------------------------

void LauncherView::select_path(GtkTreePath* path)
{
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	gtk_tree_selection_select_path(selection, path);
}

//-----------------------------------------------------------------------------

void LauncherView::set_cursor(GtkTreePath* path)
{
	gtk_tree_view_set_cursor(m_view, path, NULL, false);
}

//-----------------------------------------------------------------------------

void LauncherView::set_fixed_height_mode(bool fixed_height)
{
	gtk_tree_view_set_fixed_height_mode(m_view, fixed_height);
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

void LauncherView::collapse_all()
{
	gtk_tree_view_collapse_all(m_view);
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
	m_model = NULL;
	gtk_tree_view_set_model(m_view, NULL);
}

//-----------------------------------------------------------------------------

void LauncherView::reload_icon_size()
{
	// Force exo to reload SVG icons
	if (m_icon_size != wm_settings->launcher_icon_size.get_size())
	{
		gtk_tree_view_remove_column(m_view, m_column);
		create_column();
	}
}

//-----------------------------------------------------------------------------

void LauncherView::create_column()
{
	m_icon_size = wm_settings->launcher_icon_size.get_size();

	m_column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_expand(m_column, true);
	gtk_tree_view_column_set_visible(m_column, true);

	if (m_icon_size > 1)
	{
		GtkCellRenderer* icon_renderer = exo_cell_renderer_icon_new();
		g_object_set(icon_renderer, "follow-state", false, NULL);
		g_object_set(icon_renderer, "size", m_icon_size, NULL);
		gtk_tree_view_column_pack_start(m_column, icon_renderer, false);
		gtk_tree_view_column_add_attribute(m_column, icon_renderer, "icon", LauncherView::COLUMN_ICON);
	}

	GtkCellRenderer* text_renderer = gtk_cell_renderer_text_new();
	g_object_set(text_renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_pack_start(m_column, text_renderer, true);
	gtk_tree_view_column_add_attribute(m_column, text_renderer, "markup", LauncherView::COLUMN_TEXT);

	gtk_tree_view_column_set_sizing(m_column, GTK_TREE_VIEW_COLUMN_FIXED);

	gtk_tree_view_append_column(m_view, m_column);
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
