/*
 * Copyright (C) 2013, 2015, 2016, 2017, 2018 Graeme Gott <graeme@gottcode.org>
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

#include "configuration-dialog.h"

#include "command.h"
#include "command-edit.h"
#include "icon-size.h"
#include "plugin.h"
#include "search-action.h"
#include "settings.h"
#include "slot.h"

#include <algorithm>

#include <exo/exo.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

namespace
{

enum
{
	COLUMN_NAME,
	COLUMN_PATTERN,
	COLUMN_ACTION,
	N_COLUMNS
};

}

//-----------------------------------------------------------------------------

static void whiskermenu_config_dialog_delete(ConfigurationDialog* dialog)
{
	delete dialog;
}

//-----------------------------------------------------------------------------

ConfigurationDialog::ConfigurationDialog(Plugin* plugin) :
	m_plugin(plugin)
{
	// Create dialog window
	GtkWindow* window = NULL;
	GtkWidget* toplevel = gtk_widget_get_toplevel(m_plugin->get_button());
	if (gtk_widget_is_toplevel(toplevel))
	{
		window = GTK_WINDOW(toplevel);
	}
	m_window = xfce_titled_dialog_new_with_buttons(_("Whisker Menu"),
			window,
			GtkDialogFlags(0),
			_("_Help"), GTK_RESPONSE_HELP,
			_("_Close"), GTK_RESPONSE_CLOSE,
			NULL);
	gtk_window_set_icon_name(GTK_WINDOW(m_window), "document-properties");
	gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
	g_signal_connect_slot(m_window, "response", &ConfigurationDialog::response, this);
	g_signal_connect_swapped(m_window, "destroy", G_CALLBACK(whiskermenu_config_dialog_delete), this);

	// Create tabs
	GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_notebook_append_page(notebook, init_appearance_tab(), gtk_label_new_with_mnemonic(_("_Appearance")));
	gtk_notebook_append_page(notebook, init_behavior_tab(), gtk_label_new_with_mnemonic(_("_Behavior")));
	gtk_notebook_append_page(notebook, init_commands_tab(), gtk_label_new_with_mnemonic(_("_Commands")));
	gtk_notebook_append_page(notebook, init_search_actions_tab(), gtk_label_new_with_mnemonic(_("Search Actio_ns")));

	// Add tabs to dialog
	GtkBox* vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 8));
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);
	gtk_box_pack_start(vbox, GTK_WIDGET(notebook), true, true, 0);
	GtkBox* contents = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(m_window)));
	gtk_box_pack_start(contents, GTK_WIDGET(vbox), true, true, 0);

	// Show GTK window
	gtk_widget_show_all(m_window);

	m_plugin->set_configure_enabled(false);
}

//-----------------------------------------------------------------------------

ConfigurationDialog::~ConfigurationDialog()
{
	for (std::vector<CommandEdit*>::size_type i = 0; i < m_commands.size(); ++i)
	{
		delete m_commands[i];
	}

	g_object_unref(m_actions_model);

	m_plugin->set_configure_enabled(true);
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::choose_icon()
{
	GtkWidget* chooser = exo_icon_chooser_dialog_new(_("Select An Icon"),
			GTK_WINDOW(m_window),
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_OK"), GTK_RESPONSE_ACCEPT,
			NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(chooser), GTK_RESPONSE_ACCEPT);
	exo_icon_chooser_dialog_set_icon(EXO_ICON_CHOOSER_DIALOG(chooser), m_plugin->get_button_icon_name().c_str());

	if (gtk_dialog_run(GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
		gchar* icon = exo_icon_chooser_dialog_get_icon(EXO_ICON_CHOOSER_DIALOG(chooser));
		xfce_panel_image_set_from_source(XFCE_PANEL_IMAGE(m_icon), icon);
		m_plugin->set_button_icon_name(icon);
		g_free(icon);
	}

	gtk_widget_destroy(chooser);
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::category_icon_size_changed(GtkComboBox* combo)
{
	wm_settings->category_icon_size = gtk_combo_box_get_active(combo) - 1;
	wm_settings->set_modified();

	bool active = wm_settings->category_icon_size != -1;
	gtk_widget_set_sensitive(m_show_category_names, active);
	if (!active)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_category_names), true);
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::item_icon_size_changed(GtkComboBox* combo)
{
	wm_settings->launcher_icon_size = gtk_combo_box_get_active(combo) - 1;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::style_changed(GtkComboBox* combo)
{
	m_plugin->set_button_style(Plugin::ButtonStyle(gtk_combo_box_get_active(combo) + 1));
	gtk_widget_set_sensitive(m_button_single_row, gtk_combo_box_get_active(combo) == 0);
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::title_changed(GtkEditable* editable)
{
	const gchar* text = gtk_entry_get_text(GTK_ENTRY(editable));
	m_plugin->set_button_title(text ? text : "");
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_button_single_row(GtkToggleButton* button)
{
	wm_settings->button_single_row = gtk_toggle_button_get_active(button);
	m_plugin->set_button_style(Plugin::ButtonStyle(gtk_combo_box_get_active(GTK_COMBO_BOX(m_button_style)) + 1));
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_hover_switch_category(GtkToggleButton* button)
{
	wm_settings->category_hover_activate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_show_generic_name(GtkToggleButton* button)
{
	wm_settings->launcher_show_name = !gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_show_category_name(GtkToggleButton* button)
{
	wm_settings->category_show_name = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_show_description(GtkToggleButton* button)
{
	wm_settings->launcher_show_description = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_show_tooltip(GtkToggleButton* button)
{
	wm_settings->launcher_show_tooltip = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_show_hierarchy(GtkToggleButton* button)
{
	wm_settings->load_hierarchy = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_position_search_alternate(GtkToggleButton* button)
{
	bool active = gtk_toggle_button_get_active(button);
	wm_settings->position_search_alternate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
	gtk_widget_set_sensitive(GTK_WIDGET(m_position_commands_alternate), active);
	if (!active)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_commands_alternate), false);
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_position_commands_alternate(GtkToggleButton* button)
{
	wm_settings->position_commands_alternate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_position_categories_alternate(GtkToggleButton* button)
{
	wm_settings->position_categories_alternate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::recent_items_max_changed(GtkSpinButton* button)
{
	wm_settings->recent_items_max = gtk_spin_button_get_value_as_int(button);
	wm_settings->set_modified();
	const bool active = wm_settings->recent_items_max;
	gtk_widget_set_sensitive(GTK_WIDGET(m_display_recent), active);
	if (!active)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_display_recent), false);
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_remember_favorites(GtkToggleButton* button)
{
	wm_settings->favorites_in_recent = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_display_recent(GtkToggleButton* button)
{
	wm_settings->display_recent = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::background_opacity_changed(GtkRange* range)
{
	wm_settings->menu_opacity = gtk_range_get_value(range);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

SearchAction* ConfigurationDialog::get_selected_action(GtkTreeIter* iter) const
{
	GtkTreeIter selected_iter;
	if (!iter)
	{
		iter = &selected_iter;
	}

	SearchAction* action = NULL;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_actions_view);
	GtkTreeModel* model = NULL;
	if (gtk_tree_selection_get_selected(selection, &model, iter))
	{
		gtk_tree_model_get(model, iter, COLUMN_ACTION, &action, -1);
	}
	return action;
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::action_selected(GtkTreeView*)
{
	SearchAction* action = get_selected_action();
	if (action)
	{
		gtk_entry_set_text(GTK_ENTRY(m_action_name), action->get_name());
		gtk_entry_set_text(GTK_ENTRY(m_action_pattern), action->get_pattern());
		gtk_entry_set_text(GTK_ENTRY(m_action_command), action->get_command());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_action_regex), action->get_is_regex());
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::action_name_changed(GtkEditable* editable)
{
	GtkTreeIter iter;
	SearchAction* action = get_selected_action(&iter);
	if (action)
	{
		const gchar* text = gtk_entry_get_text(GTK_ENTRY(editable));
		action->set_name(text);
		gtk_list_store_set(m_actions_model, &iter, COLUMN_NAME, text, -1);
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::action_pattern_changed(GtkEditable* editable)
{
	GtkTreeIter iter;
	SearchAction* action = get_selected_action(&iter);
	if (action)
	{
		const gchar* text = gtk_entry_get_text(GTK_ENTRY(editable));
		action->set_pattern(text);
		gtk_list_store_set(m_actions_model, &iter, COLUMN_PATTERN, text, -1);
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::action_command_changed(GtkEditable* editable)
{
	SearchAction* action = get_selected_action();
	if (action)
	{
		action->set_command(gtk_entry_get_text(GTK_ENTRY(editable)));
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::action_toggle_regex(GtkToggleButton* button)
{
	SearchAction* action = get_selected_action();
	if (action)
	{
		action->set_is_regex(gtk_toggle_button_get_active(button));
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::add_action(GtkButton*)
{
	// Add to action list
	SearchAction* action = new SearchAction;
	wm_settings->search_actions.push_back(action);
	wm_settings->set_modified();

	// Add to model
	GtkTreeIter iter;
	gtk_list_store_insert_with_values(m_actions_model, &iter, G_MAXINT,
			COLUMN_NAME, "",
			COLUMN_PATTERN, "",
			COLUMN_ACTION, action,
			-1);
	GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(m_actions_model), &iter);
	gtk_tree_view_set_cursor(m_actions_view, path, NULL, false);
	gtk_tree_path_free(path);

	// Make sure editing is allowed
	gtk_widget_set_sensitive(m_action_remove, true);
	gtk_widget_set_sensitive(m_action_name, true);
	gtk_widget_set_sensitive(m_action_pattern, true);
	gtk_widget_set_sensitive(m_action_command, true);
	gtk_widget_set_sensitive(m_action_regex, true);
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::remove_action(GtkButton* button)
{
	// Fetch action
	GtkTreeIter iter;
	SearchAction* action = get_selected_action(&iter);
	if (!action)
	{
		return;
	}

	// Confirm removal
	if (!xfce_dialog_confirm(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
			"edit-delete", _("_Delete"),
			_("The action will be deleted permanently."),
			_("Remove action \"%s\"?"),
			action->get_name()))
	{
		return;
	}

	// Fetch path of previous action
	GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(m_actions_model), &iter);
	if (!gtk_tree_path_prev(path))
	{
		gtk_tree_path_free(path);
		path = NULL;
	}

	// Remove from model
	if (gtk_list_store_remove(m_actions_model, &iter))
	{
		if (path)
		{
			gtk_tree_path_free(path);
		}
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(m_actions_model), &iter);
	}

	// Remove from list
	wm_settings->search_actions.erase(std::find(wm_settings->search_actions.begin(), wm_settings->search_actions.end(), action));
	wm_settings->set_modified();
	delete action;

	// Select next action
	if (path)
	{
		gtk_tree_view_set_cursor(m_actions_view, path, NULL, false);
		gtk_tree_path_free(path);
	}
	else
	{
		gtk_entry_set_text(GTK_ENTRY(m_action_name), "");
		gtk_entry_set_text(GTK_ENTRY(m_action_pattern), "");
		gtk_entry_set_text(GTK_ENTRY(m_action_command), "");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_action_regex), false);

		gtk_widget_set_sensitive(m_action_remove, false);
		gtk_widget_set_sensitive(m_action_name, false);
		gtk_widget_set_sensitive(m_action_pattern, false);
		gtk_widget_set_sensitive(m_action_command, false);
		gtk_widget_set_sensitive(m_action_regex, false);
	}
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::response(GtkDialog*, int response_id)
{
	if (response_id == GTK_RESPONSE_HELP)
	{
		bool result = g_spawn_command_line_async("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

		if (G_UNLIKELY(result == false))
		{
			g_warning(_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
		}
	}
	else
	{
		if ((m_plugin->get_button_style() == Plugin::ShowText) && m_plugin->get_button_title().empty())
		{
			m_plugin->set_button_title(Plugin::get_button_title_default());
		}

		for (int i = 0; i < Settings::CountCommands; ++i)
		{
			wm_settings->command[i]->check();
		}

		if (response_id == GTK_RESPONSE_CLOSE)
		{
			gtk_widget_destroy(m_window);
		}
	}
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_appearance_tab()
{
	// Create appearance page
	GtkBox* page = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);


	// Create panel button section
	GtkGrid* panel_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(panel_table, 12);
	gtk_grid_set_row_spacing(panel_table, 6);

	GtkWidget* panel_frame = xfce_gtk_frame_box_new_with_content(_("Panel Button"), GTK_WIDGET(panel_table));
	gtk_box_pack_start(page, panel_frame, false, false, 6);
	gtk_container_set_border_width(GTK_CONTAINER(panel_frame), 0);

	// Add button style selector
	GtkWidget* label = gtk_label_new_with_mnemonic(_("Di_splay:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(panel_table, label, 0, 0, 1, 1);

	m_button_style = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Icon"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Title"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Icon and title"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_button_style), static_cast<int>(m_plugin->get_button_style()) - 1);
	gtk_widget_set_hexpand(GTK_WIDGET(m_button_style), true);
	gtk_grid_attach(panel_table, m_button_style, 1, 0, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_button_style);
	g_signal_connect_slot(m_button_style, "changed", &ConfigurationDialog::style_changed, this);

	// Add title selector
	label = gtk_label_new_with_mnemonic(_("_Title:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(panel_table, label, 0, 1, 1, 1);

	m_title = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_title), m_plugin->get_button_title().c_str());
	gtk_grid_attach(panel_table, m_title, 1, 1, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_title);
	g_signal_connect_slot(m_title, "changed", &ConfigurationDialog::title_changed, this);

	// Add icon selector
	label = gtk_label_new_with_mnemonic(_("_Icon:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(panel_table, label, 0, 2, 1, 1);

	m_icon_button = gtk_button_new();
	gtk_widget_set_halign(m_icon_button, GTK_ALIGN_START);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_icon_button);
	g_signal_connect_slot<GtkButton*>(m_icon_button, "clicked", &ConfigurationDialog::choose_icon, this);
	gtk_grid_attach(panel_table, m_icon_button, 1, 2, 1, 1);

	m_icon = xfce_panel_image_new_from_source(m_plugin->get_button_icon_name().c_str());
	xfce_panel_image_set_size(XFCE_PANEL_IMAGE(m_icon), 48);
	gtk_container_add(GTK_CONTAINER(m_icon_button), m_icon);

	m_button_single_row = gtk_check_button_new_with_mnemonic(_("Use a single _panel row"));
	gtk_grid_attach(panel_table, m_button_single_row, 1, 3, 1, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button_single_row), wm_settings->button_single_row);
	gtk_widget_set_sensitive(m_button_single_row, gtk_combo_box_get_active(GTK_COMBO_BOX (m_button_style)) == 0);
	g_signal_connect_slot(m_button_single_row, "toggled", &ConfigurationDialog::toggle_button_single_row, this);
	gtk_widget_show(m_button_single_row);


	// Create menu section
	GtkGrid* menu_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(menu_table, 12);
	gtk_grid_set_row_spacing(menu_table, 6);

	GtkWidget* appearance_frame = xfce_gtk_frame_box_new_with_content(_("Menu"), GTK_WIDGET(menu_table));
	gtk_box_pack_start(page, appearance_frame, false, false, 6);
	gtk_container_set_border_width(GTK_CONTAINER(appearance_frame), 0);

	// Add option to use generic names
	m_show_generic_names = gtk_check_button_new_with_mnemonic(_("Show generic application _names"));
	gtk_grid_attach(menu_table, m_show_generic_names, 0, 0, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_generic_names), !wm_settings->launcher_show_name);
	g_signal_connect_slot(m_show_generic_names, "toggled", &ConfigurationDialog::toggle_show_generic_name, this);

	// Add option to hide category names
	m_show_category_names = gtk_check_button_new_with_mnemonic(_("Show cate_gory names"));
	gtk_grid_attach(menu_table, m_show_category_names, 0, 1, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_category_names), wm_settings->category_show_name);
	gtk_widget_set_sensitive(m_show_category_names, wm_settings->category_icon_size != -1);
	g_signal_connect_slot(m_show_category_names, "toggled", &ConfigurationDialog::toggle_show_category_name, this);

	// Add option to hide descriptions
	m_show_descriptions = gtk_check_button_new_with_mnemonic(_("Show application _descriptions"));
	gtk_grid_attach(menu_table, m_show_descriptions, 0, 2, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_descriptions), wm_settings->launcher_show_description);
	g_signal_connect_slot(m_show_descriptions, "toggled", &ConfigurationDialog::toggle_show_description, this);

	// Add option to hide tooltips
	m_show_tooltips = gtk_check_button_new_with_mnemonic(_("Show application too_ltips"));
	gtk_grid_attach(menu_table, m_show_tooltips, 0, 3, 3, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_tooltips), wm_settings->launcher_show_tooltip);
	g_signal_connect_slot(m_show_tooltips, "toggled", &ConfigurationDialog::toggle_show_tooltip, this);

	// Add option to show menu hierarchy
	m_show_hierarchy = gtk_check_button_new_with_mnemonic(_("Show menu hie_rarchy"));
	gtk_grid_attach(menu_table, m_show_hierarchy, 0, 4, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_hierarchy), wm_settings->load_hierarchy);
	g_signal_connect_slot(m_show_hierarchy, "toggled", &ConfigurationDialog::toggle_show_hierarchy, this);

	// Add item icon size selector
	label = gtk_label_new_with_mnemonic(_("Ite_m icon size:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(menu_table, label, 0, 5, 1, 1);

	m_item_icon_size = gtk_combo_box_text_new();
	std::vector<std::string> icon_sizes = IconSize::get_strings();
	for (std::vector<std::string>::const_iterator i = icon_sizes.begin(), end = icon_sizes.end(); i != end; ++i)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_item_icon_size), i->c_str());
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_item_icon_size), wm_settings->launcher_icon_size + 1);
	gtk_grid_attach(menu_table, m_item_icon_size, 1, 5, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_item_icon_size);
	g_signal_connect_slot(m_item_icon_size, "changed", &ConfigurationDialog::item_icon_size_changed, this);

	// Add category icon size selector
	label = gtk_label_new_with_mnemonic(_("Categ_ory icon size:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(menu_table, label, 0, 6, 1, 1);

	m_category_icon_size = gtk_combo_box_text_new();
	for (std::vector<std::string>::const_iterator i = icon_sizes.begin(), end = icon_sizes.end(); i != end; ++i)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_category_icon_size), i->c_str());
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_category_icon_size), wm_settings->category_icon_size + 1);
	gtk_grid_attach(menu_table, m_category_icon_size, 1, 6, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_category_icon_size);
	g_signal_connect_slot(m_category_icon_size, "changed", &ConfigurationDialog::category_icon_size_changed, this);

	// Add option to control background opacity
	label = gtk_label_new_with_mnemonic(_("Background opacit_y:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(menu_table, label, 0, 7, 1, 1);

	m_background_opacity = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
	gtk_widget_set_hexpand(GTK_WIDGET(m_background_opacity), true);
	gtk_grid_attach(menu_table, m_background_opacity, 1, 7, 1, 1);
	gtk_scale_set_value_pos(GTK_SCALE(m_background_opacity), GTK_POS_RIGHT);
	gtk_range_set_value(GTK_RANGE(m_background_opacity), wm_settings->menu_opacity);
	g_signal_connect_slot(m_background_opacity, "value-changed", &ConfigurationDialog::background_opacity_changed, this);

	GdkScreen* screen = gtk_widget_get_screen(m_window);
	const bool enabled = gdk_screen_is_composited(screen);
	gtk_widget_set_sensitive(label, enabled);
	gtk_widget_set_sensitive(GTK_WIDGET(m_background_opacity), enabled);

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_behavior_tab()
{
	// Create behavior page
	GtkBox* page = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 8));
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);


	// Create menu section
	GtkBox* behavior_vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	GtkWidget* behavior_frame = xfce_gtk_frame_box_new_with_content(_("Menu"), GTK_WIDGET(behavior_vbox));
	gtk_box_pack_start(page, behavior_frame, false, false, 6);
	gtk_container_set_border_width(GTK_CONTAINER(behavior_frame), 0);

	// Add option to switch categories by hovering
	m_hover_switch_category = gtk_check_button_new_with_mnemonic(_("Switch categories by _hovering"));
	gtk_box_pack_start(behavior_vbox, m_hover_switch_category, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_hover_switch_category), wm_settings->category_hover_activate);
	g_signal_connect_slot(m_hover_switch_category, "toggled", &ConfigurationDialog::toggle_hover_switch_category, this);

	// Add option to use alternate search entry position
	m_position_search_alternate = gtk_check_button_new_with_mnemonic(_("Position _search entry next to panel button"));
	gtk_box_pack_start(behavior_vbox, m_position_search_alternate, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_search_alternate), wm_settings->position_search_alternate);
	g_signal_connect_slot(m_position_search_alternate, "toggled", &ConfigurationDialog::toggle_position_search_alternate, this);

	// Add option to use alternate commands position
	m_position_commands_alternate = gtk_check_button_new_with_mnemonic(_("Position commands next to search _entry"));
	gtk_box_pack_start(behavior_vbox, m_position_commands_alternate, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_commands_alternate), wm_settings->position_commands_alternate);
	gtk_widget_set_sensitive(GTK_WIDGET(m_position_commands_alternate), wm_settings->position_search_alternate);
	g_signal_connect_slot(m_position_commands_alternate, "toggled", &ConfigurationDialog::toggle_position_commands_alternate, this);

	// Add option to use alternate categories position
	m_position_categories_alternate = gtk_check_button_new_with_mnemonic(_("Position cate_gories next to panel button"));
	gtk_box_pack_start(behavior_vbox, m_position_categories_alternate, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_categories_alternate), wm_settings->position_categories_alternate);
	g_signal_connect_slot(m_position_categories_alternate, "toggled", &ConfigurationDialog::toggle_position_categories_alternate, this);


	// Create recently used section
	GtkGrid* recent_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(recent_table, 12);
	gtk_grid_set_row_spacing(recent_table, 6);

	GtkWidget* recent_frame = xfce_gtk_frame_box_new_with_content(_("Recently Used"), GTK_WIDGET(recent_table));
	gtk_box_pack_start(page, recent_frame, false, false, 6);
	gtk_container_set_border_width(GTK_CONTAINER(recent_frame), 0);

	// Add value to change maximum number of recently used entries
	GtkWidget* label = gtk_label_new_with_mnemonic(_("Amount of _items:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(recent_table, label, 0, 0, 1, 1);

	m_recent_items_max = gtk_spin_button_new_with_range(0, 100, 1);
	gtk_grid_attach(recent_table, m_recent_items_max, 1, 0, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_recent_items_max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_recent_items_max), wm_settings->recent_items_max);
	g_signal_connect_slot(m_recent_items_max, "value-changed", &ConfigurationDialog::recent_items_max_changed, this);

	// Add option to remember favorites
	m_remember_favorites = gtk_check_button_new_with_mnemonic(_("Include _favorites"));
	gtk_grid_attach(recent_table, m_remember_favorites, 0, 1, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_remember_favorites), wm_settings->favorites_in_recent);
	g_signal_connect_slot(m_remember_favorites, "toggled", &ConfigurationDialog::toggle_remember_favorites, this);

	// Add option to display recently used
	m_display_recent = gtk_check_button_new_with_mnemonic(_("Display by _default"));
	gtk_grid_attach(recent_table, m_display_recent, 0, 2, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_display_recent), wm_settings->display_recent);
	gtk_widget_set_sensitive(GTK_WIDGET(m_display_recent), wm_settings->recent_items_max);
	g_signal_connect_slot(m_display_recent, "toggled", &ConfigurationDialog::toggle_display_recent, this);

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_commands_tab()
{
	// Create commands page
	GtkBox* page = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 8));
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);
	GtkSizeGroup* label_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	// Add command entries
	for (int i = 0; i < Settings::CountCommands; ++i)
	{
		CommandEdit* command_edit = new CommandEdit(wm_settings->command[i], label_size_group);
		gtk_box_pack_start(page, command_edit->get_widget(), false, false, 0);
		m_commands.push_back(command_edit);
	}

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_search_actions_tab()
{
	// Create search actions page
	GtkGrid* page = GTK_GRID(gtk_grid_new());
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);
	gtk_grid_set_column_spacing(page, 6);
	gtk_grid_set_row_spacing(page, 6);

	// Create model
	m_actions_model = gtk_list_store_new(N_COLUMNS,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_POINTER);
	for (std::vector<SearchAction*>::size_type i = 0, end = wm_settings->search_actions.size(); i < end; ++i)
	{
		SearchAction* action = wm_settings->search_actions[i];
		gtk_list_store_insert_with_values(m_actions_model,
				NULL, G_MAXINT,
				COLUMN_NAME, action->get_name(),
				COLUMN_PATTERN, action->get_pattern(),
				COLUMN_ACTION, action,
				-1);
	}

	// Create view
	m_actions_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_actions_model)));
	g_signal_connect_slot(m_actions_view, "cursor-changed", &ConfigurationDialog::action_selected, this);

	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Name"),
			renderer, "text", COLUMN_NAME, NULL);
	gtk_tree_view_append_column(m_actions_view, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Pattern"),
			renderer, "text", COLUMN_PATTERN, NULL);
	gtk_tree_view_append_column(m_actions_view, column);

	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_actions_view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);

	GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(m_actions_view));
	gtk_widget_set_hexpand(GTK_WIDGET(scrolled_window), true);
	gtk_widget_set_vexpand(GTK_WIDGET(scrolled_window), true);
	gtk_grid_attach(page, scrolled_window, 0, 0, 1, 1);

	// Create buttons
	m_action_add = gtk_button_new();
	gtk_widget_set_tooltip_text(m_action_add, _("Add action"));
	gtk_widget_show(m_action_add);

	GtkWidget* image = gtk_image_new_from_icon_name("list-add", GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(m_action_add), image);
	gtk_widget_show(image);
	g_signal_connect_slot(m_action_add, "clicked", &ConfigurationDialog::add_action, this);

	m_action_remove = gtk_button_new();
	gtk_widget_set_tooltip_text(m_action_remove, _("Remove selected action"));
	gtk_widget_show(m_action_remove);

	image = gtk_image_new_from_icon_name("list-remove", GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(m_action_remove), image);
	gtk_widget_show(image);
	g_signal_connect_slot(m_action_remove, "clicked", &ConfigurationDialog::remove_action, this);

	GtkBox* actions_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	gtk_widget_set_halign(GTK_WIDGET(actions_box), GTK_ALIGN_START);
	gtk_box_pack_start(actions_box, m_action_add, false, false, 0);
	gtk_box_pack_start(actions_box, m_action_remove, false, false, 0);
	gtk_grid_attach(page, GTK_WIDGET(actions_box), 1, 0, 1, 1);
	gtk_widget_show_all(GTK_WIDGET(actions_box));

	// Create details section
	GtkGrid* details_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(details_table, 6);
	gtk_grid_set_row_spacing(details_table, 6);
	GtkWidget* details_frame = xfce_gtk_frame_box_new_with_content(_("Details"), GTK_WIDGET(details_table));
	gtk_grid_attach(page, details_frame, 0, 1, 2, 1);
	gtk_container_set_border_width(GTK_CONTAINER(details_frame), 0);

	// Create entry for name
	GtkWidget* label = gtk_label_new_with_mnemonic(_("Nam_e:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_widget_show(label);
	gtk_grid_attach(details_table, label, 0, 0, 1, 1);

	m_action_name = gtk_entry_new();
	gtk_widget_show(m_action_name);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_action_name);
	gtk_widget_set_hexpand(m_action_name, true);
	gtk_grid_attach(details_table, m_action_name, 1, 0, 1, 1);
	g_signal_connect_slot(m_action_name, "changed", &ConfigurationDialog::action_name_changed, this);

	// Create entry for keyword
	label = gtk_label_new_with_mnemonic(_("_Pattern:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_widget_show(label);
	gtk_grid_attach(details_table, label, 0, 1, 1, 1);

	m_action_pattern = gtk_entry_new();
	gtk_widget_show(m_action_pattern);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_action_pattern);
	gtk_grid_attach(details_table, m_action_pattern, 1, 1, 1, 1);
	g_signal_connect_slot(m_action_pattern, "changed", &ConfigurationDialog::action_pattern_changed, this);

	// Create entry for command
	label = gtk_label_new_with_mnemonic(_("C_ommand:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_widget_show(label);
	gtk_grid_attach(details_table, label, 0, 2, 1, 1);

	m_action_command = gtk_entry_new();
	gtk_widget_show(m_action_command);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_action_command);
	gtk_grid_attach(details_table, m_action_command, 1, 2, 1, 1);
	g_signal_connect_slot(m_action_command, "changed", &ConfigurationDialog::action_command_changed, this);

	// Create toggle button for regular expressions
	m_action_regex = gtk_check_button_new_with_mnemonic(_("_Regular expression"));
	gtk_widget_show(m_action_regex);
	gtk_grid_attach(details_table, m_action_regex, 1, 3, 1, 1);
	g_signal_connect_slot(m_action_regex, "toggled", &ConfigurationDialog::action_toggle_regex, this);

	// Select first action
	if (!wm_settings->search_actions.empty())
	{
		GtkTreePath* path = gtk_tree_path_new_first();
		gtk_tree_view_set_cursor(m_actions_view, path, NULL, false);
		gtk_tree_path_free(path);
	}
	else
	{
		gtk_widget_set_sensitive(m_action_remove, false);
		gtk_widget_set_sensitive(m_action_name, false);
		gtk_widget_set_sensitive(m_action_pattern, false);
		gtk_widget_set_sensitive(m_action_command, false);
		gtk_widget_set_sensitive(m_action_regex, false);
	}

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------
