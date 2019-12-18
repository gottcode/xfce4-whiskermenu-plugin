/*
 * Copyright (C) 2019 Graeme Gott <graeme@gottcode.org>
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

#include "launcher-icon-view.h"

#include "settings.h"
#include "slot.h"

#include <exo/exo.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

LauncherIconView::LauncherIconView() :
	m_icon_renderer(NULL),
	m_icon_size(-1)
{
	// Create the view
	m_view = GTK_ICON_VIEW(gtk_icon_view_new());

	m_icon_renderer = exo_cell_renderer_icon_new();
	g_object_set(m_icon_renderer,
			"follow-state", false,
			"xalign", 0.5,
			"yalign", 1.0,
			NULL);
	GtkCellLayout* cell_layout = GTK_CELL_LAYOUT(m_view);
	gtk_cell_layout_pack_start(cell_layout, m_icon_renderer, false);
	gtk_cell_layout_set_attributes(cell_layout, m_icon_renderer, "icon", COLUMN_ICON, NULL);

	gtk_icon_view_set_markup_column(m_view, COLUMN_TEXT);

	reload_icon_size();

	// Use single clicks to activate items
	gtk_icon_view_set_activate_on_single_click(m_view, true);

	// Only allow up to one selected item
	gtk_icon_view_set_selection_mode(m_view, GTK_SELECTION_SINGLE);

	g_object_ref_sink(m_view);
}

//-----------------------------------------------------------------------------

LauncherIconView::~LauncherIconView()
{
	gtk_widget_destroy(GTK_WIDGET(m_view));
	g_object_unref(m_view);
}

//-----------------------------------------------------------------------------

GtkTreePath* LauncherIconView::get_cursor() const
{
	GtkTreePath* path = NULL;
	gtk_icon_view_get_cursor(m_view, &path, NULL);
	return path;
}

//-----------------------------------------------------------------------------

GtkTreePath* LauncherIconView::get_path_at_pos(int x, int y) const
{
	return gtk_icon_view_get_path_at_pos(m_view, x, y);
}

//-----------------------------------------------------------------------------

GtkTreePath* LauncherIconView::get_selected_path() const
{
	GtkTreePath* path = NULL;
	GList* selection = gtk_icon_view_get_selected_items(m_view);
	if (selection != NULL)
	{
		path = gtk_tree_path_copy(reinterpret_cast<GtkTreePath*>(selection->data));
	}
	g_list_free_full(selection, (GDestroyNotify)gtk_tree_path_free);
	return path;
}

//-----------------------------------------------------------------------------

void LauncherIconView::activate_path(GtkTreePath* path)
{
	gtk_icon_view_item_activated(m_view, path);
}

//-----------------------------------------------------------------------------

void LauncherIconView::scroll_to_path(GtkTreePath* path)
{
	gtk_icon_view_scroll_to_path(m_view, path, true, 0.5f, 0.5f);
}

//-----------------------------------------------------------------------------

void LauncherIconView::select_path(GtkTreePath* path)
{
	gtk_icon_view_select_path(m_view, path);
}

//-----------------------------------------------------------------------------

void LauncherIconView::set_cursor(GtkTreePath* path)
{
	gtk_icon_view_set_cursor(m_view,path, NULL, false);
}

//-----------------------------------------------------------------------------

void LauncherIconView::set_fixed_height_mode(bool)
{
}

//-----------------------------------------------------------------------------

void LauncherIconView::set_selection_mode(GtkSelectionMode mode)
{
	gtk_icon_view_set_selection_mode(m_view, mode);
}

//-----------------------------------------------------------------------------

void LauncherIconView::hide_tooltips()
{
	gtk_icon_view_set_tooltip_column(m_view, -1);
}

//-----------------------------------------------------------------------------

void LauncherIconView::show_tooltips()
{
	gtk_icon_view_set_tooltip_column(m_view, COLUMN_TOOLTIP);
}

//-----------------------------------------------------------------------------

void LauncherIconView::clear_selection()
{
	gtk_icon_view_unselect_all(m_view);
}

//-----------------------------------------------------------------------------

void LauncherIconView::collapse_all()
{
}

//-----------------------------------------------------------------------------

void LauncherIconView::set_model(GtkTreeModel* model)
{
	m_model = model;
	gtk_icon_view_set_model(m_view, model);
}

//-----------------------------------------------------------------------------

void LauncherIconView::unset_model()
{
	m_model = NULL;
	gtk_icon_view_set_model(m_view, NULL);
}

//-----------------------------------------------------------------------------

void LauncherIconView::set_drag_source(GdkModifierType start_button_mask, const GtkTargetEntry* targets, gint n_targets, GdkDragAction actions)
{
	gtk_icon_view_enable_model_drag_source(m_view, start_button_mask, targets, n_targets, actions);
}

//-----------------------------------------------------------------------------

void LauncherIconView::set_drag_dest(const GtkTargetEntry* targets, gint n_targets, GdkDragAction actions)
{
	gtk_icon_view_enable_model_drag_dest(m_view, targets, n_targets, actions);
}

//-----------------------------------------------------------------------------

void LauncherIconView::unset_drag_source()
{
	gtk_icon_view_unset_model_drag_source(m_view);
}

//-----------------------------------------------------------------------------

void LauncherIconView::unset_drag_dest()
{
	gtk_icon_view_unset_model_drag_dest(m_view);
}

//-----------------------------------------------------------------------------

void LauncherIconView::reload_icon_size()
{
	// Fetch icon size
	if (m_icon_size == wm_settings->launcher_icon_size.get_size())
	{
		return;
	}
	m_icon_size = wm_settings->launcher_icon_size.get_size();

	// Configure icon renderer
	if (m_icon_size > 1)
	{
		g_object_set(m_icon_renderer, "size", m_icon_size, "visible", true, NULL);
	}
	else
	{
		g_object_set(m_icon_renderer, "visible", false, NULL);
	}

	// Adjust item size
	int padding = 2;
	int width = 88;
	switch (wm_settings->launcher_icon_size)
	{
	case IconSize::Smallest:
		padding = 2;
		width = 88;
		break;

	case IconSize::Smaller:
		padding = 2;
		width =  92;
		break;

	case IconSize::Small:
		padding = 4;
		width = 100;
		break;

	case IconSize::Normal:
		padding = 4;
		width = 108;
		break;

	case IconSize::Large:
		padding = 4;
		width = 116;
		break;

	case IconSize::Larger:
		padding = 6;
		width = 136;
		break;

	case IconSize::Largest:
		padding = 6;
		width = 152;
		break;

	default:
		break;
	}
	gtk_icon_view_set_item_padding(m_view, padding);
	gtk_icon_view_set_item_width(m_view, width);
}

//-----------------------------------------------------------------------------
