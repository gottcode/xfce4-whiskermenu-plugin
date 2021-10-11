/*
 * Copyright (C) 2013-2021 Graeme Gott <graeme@gottcode.org>
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

#include "settings-dialog.h"

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

static void settings_dialog_free(GtkWidget*, gpointer user_data)
{
	delete static_cast<SettingsDialog*>(user_data);
}

//-----------------------------------------------------------------------------

static GtkWidget* make_aligned_frame(const gchar* text, GtkWidget* content)
{
	// Create bold label
	gchar* markup = g_markup_printf_escaped("<b>%s</b>", text);
	GtkWidget* label = gtk_label_new(nullptr);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	g_free(markup);

	// Create frame
	GtkWidget* frame = gtk_frame_new(nullptr);
	gtk_frame_set_label_widget(GTK_FRAME(frame), label);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);

	// Add content
	gtk_widget_set_margin_start(content, 12);
	gtk_widget_set_margin_top(content, 6);
	gtk_container_add(GTK_CONTAINER(frame), content);

	return frame;
}

//-----------------------------------------------------------------------------

SettingsDialog::SettingsDialog(Plugin* plugin) :
	m_plugin(plugin)
{
	// Create dialog window
	GtkWindow* window = nullptr;
	GtkWidget* toplevel = gtk_widget_get_toplevel(m_plugin->get_button());
	if (gtk_widget_is_toplevel(toplevel))
	{
		window = GTK_WINDOW(toplevel);
	}
#if LIBXFCE4PANEL_CHECK_VERSION(4,13,0)
	m_window = xfce_titled_dialog_new_with_mixed_buttons(_("Whisker Menu"),
			window,
			GtkDialogFlags(0),
			"help-browser", _("_Help"), GTK_RESPONSE_HELP,
			"window-close-symbolic", _("_Close"), GTK_RESPONSE_CLOSE,
			nullptr);
#else
	m_window = xfce_titled_dialog_new_with_buttons(_("Whisker Menu"),
			window,
			GtkDialogFlags(0),
			_("_Help"), GTK_RESPONSE_HELP,
			_("_Close"), GTK_RESPONSE_CLOSE,
			nullptr);
#endif
	gtk_window_set_icon_name(GTK_WINDOW(m_window), "xfce4-whiskermenu");
	gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
	g_signal_connect_slot(m_window, "response", &SettingsDialog::response, this);
	g_signal_connect(m_window, "destroy", G_CALLBACK(&settings_dialog_free), this);

	// Create tabs
	GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_notebook_append_page(notebook, init_general_tab(), gtk_label_new_with_mnemonic(_("_General")));
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

SettingsDialog::~SettingsDialog()
{
	for (auto command : m_commands)
	{
		delete command;
	}

	g_object_unref(m_actions_model);

	m_plugin->set_configure_enabled(true);
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_show_as_icons(GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active(button))
	{
		wm_settings->view_as_icons = true;
		wm_settings->load_hierarchy = false;
		wm_settings->set_modified();
		m_plugin->reload();

		gtk_widget_set_sensitive(GTK_WIDGET(m_show_descriptions), false);
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_show_as_list(GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active(button))
	{
		wm_settings->view_as_icons = false;
		wm_settings->load_hierarchy = false;
		wm_settings->set_modified();
		m_plugin->reload();

		gtk_widget_set_sensitive(GTK_WIDGET(m_show_descriptions), true);
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_show_as_tree(GtkToggleButton* button)
{
	if (gtk_toggle_button_get_active(button))
	{
		wm_settings->view_as_icons = false;
		wm_settings->load_hierarchy = true;
		wm_settings->set_modified();
		m_plugin->reload();

		gtk_widget_set_sensitive(GTK_WIDGET(m_show_descriptions), true);
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_show_generic_name(GtkToggleButton* button)
{
	wm_settings->launcher_show_name = !gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_show_category_name(GtkToggleButton* button)
{
	wm_settings->category_show_name = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_show_description(GtkToggleButton* button)
{
	wm_settings->launcher_show_description = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_show_tooltip(GtkToggleButton* button)
{
	wm_settings->launcher_show_tooltip = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::category_icon_size_changed(GtkComboBox* combo)
{
	wm_settings->category_icon_size = gtk_combo_box_get_active(combo) - 1;
	wm_settings->set_modified();

	const bool active = (wm_settings->category_icon_size != -1) && !wm_settings->position_categories_horizontal;
	gtk_widget_set_sensitive(m_show_category_names, active);
	if (!active)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_category_names), true);
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::item_icon_size_changed(GtkComboBox* combo)
{
	wm_settings->launcher_icon_size = gtk_combo_box_get_active(combo) - 1;
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::background_opacity_changed(GtkRange* range)
{
	wm_settings->menu_opacity = gtk_range_get_value(range);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_position_categories_horizontal(GtkToggleButton* button)
{
	wm_settings->position_categories_horizontal = gtk_toggle_button_get_active(button);
	const bool active = (wm_settings->category_icon_size != -1) && !wm_settings->position_categories_horizontal;
	gtk_widget_set_sensitive(m_show_category_names, active);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_position_categories_alternate(GtkToggleButton* button)
{
	wm_settings->position_categories_alternate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_position_search_alternate(GtkToggleButton* button)
{
	wm_settings->position_search_alternate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_position_commands_alternate(GtkToggleButton* button)
{
	wm_settings->position_commands_alternate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::profile_shape_changed(GtkComboBox* combo)
{
	wm_settings->profile_shape = gtk_combo_box_get_active(combo);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::style_changed(GtkComboBox* combo)
{
	m_plugin->set_button_style(Plugin::ButtonStyle(gtk_combo_box_get_active(combo) + 1));
	gtk_widget_set_sensitive(m_button_single_row, gtk_combo_box_get_active(combo) == 0);
}

//-----------------------------------------------------------------------------

void SettingsDialog::title_changed(GtkEditable* editable)
{
	const gchar* text = gtk_entry_get_text(GTK_ENTRY(editable));
	m_plugin->set_button_title(text ? text : "");
}

//-----------------------------------------------------------------------------

void SettingsDialog::choose_icon()
{
	GtkWidget* chooser = exo_icon_chooser_dialog_new(_("Select an Icon"),
			GTK_WINDOW(m_window),
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_OK"), GTK_RESPONSE_ACCEPT,
			nullptr);

	gtk_dialog_set_default_response(GTK_DIALOG(chooser), GTK_RESPONSE_ACCEPT);
	exo_icon_chooser_dialog_set_icon(EXO_ICON_CHOOSER_DIALOG(chooser), m_plugin->get_button_icon_name().c_str());

	if (gtk_dialog_run(GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
		gchar* icon = exo_icon_chooser_dialog_get_icon(EXO_ICON_CHOOSER_DIALOG(chooser));
		gtk_image_set_from_icon_name(GTK_IMAGE(m_icon), icon, GTK_ICON_SIZE_DIALOG);
		m_plugin->set_button_icon_name(icon);
		g_free(icon);
	}

	gtk_widget_destroy(chooser);
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_button_single_row(GtkToggleButton* button)
{
	wm_settings->button_single_row = gtk_toggle_button_get_active(button);
	m_plugin->set_button_style(Plugin::ButtonStyle(gtk_combo_box_get_active(GTK_COMBO_BOX(m_button_style)) + 1));
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_hover_switch_category(GtkToggleButton* button)
{
	wm_settings->category_hover_activate = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_stay_on_focus_out(GtkToggleButton* button)
{
	wm_settings->stay_on_focus_out = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_sort_categories(GtkToggleButton* button)
{
	wm_settings->sort_categories = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_display_favorites(GtkToggleButton* button)
{
	if (gtk_toggle_button_get_active(button))
	{
		wm_settings->default_category = 0;
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_display_recent(GtkToggleButton* button)
{
	if (gtk_toggle_button_get_active(button))
	{
		wm_settings->default_category = 1;
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_display_applications(GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active(button))
	{
		wm_settings->default_category = 2;
		wm_settings->set_modified();
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::recent_items_max_changed(GtkSpinButton* button)
{
	wm_settings->recent_items_max = gtk_spin_button_get_value_as_int(button);
	wm_settings->set_modified();
	const bool active = wm_settings->recent_items_max;
	gtk_widget_set_sensitive(GTK_WIDGET(m_display_recent), active);
	if (!active && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_display_recent)))
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_display_favorites), true);
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_remember_favorites(GtkToggleButton* button)
{
	wm_settings->favorites_in_recent = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

void SettingsDialog::toggle_confirm_session_command(GtkToggleButton* button)
{
	wm_settings->confirm_session_command = gtk_toggle_button_get_active(button);
	wm_settings->set_modified();
}

//-----------------------------------------------------------------------------

SearchAction* SettingsDialog::get_selected_action(GtkTreeIter* iter) const
{
	GtkTreeIter selected_iter;
	if (!iter)
	{
		iter = &selected_iter;
	}

	SearchAction* action = nullptr;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_actions_view);
	GtkTreeModel* model = nullptr;
	if (gtk_tree_selection_get_selected(selection, &model, iter))
	{
		gtk_tree_model_get(model, iter, COLUMN_ACTION, &action, -1);
	}
	return action;
}

//-----------------------------------------------------------------------------

void SettingsDialog::action_selected(GtkTreeView*)
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

void SettingsDialog::action_name_changed(GtkEditable* editable)
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

void SettingsDialog::action_pattern_changed(GtkEditable* editable)
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

void SettingsDialog::action_command_changed(GtkEditable* editable)
{
	SearchAction* action = get_selected_action();
	if (action)
	{
		action->set_command(gtk_entry_get_text(GTK_ENTRY(editable)));
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::action_toggle_regex(GtkToggleButton* button)
{
	SearchAction* action = get_selected_action();
	if (action)
	{
		action->set_is_regex(gtk_toggle_button_get_active(button));
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::add_action(GtkButton*)
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
	gtk_tree_view_set_cursor(m_actions_view, path, nullptr, false);
	gtk_tree_path_free(path);

	// Make sure editing is allowed
	gtk_widget_set_sensitive(m_action_remove, true);
	gtk_widget_set_sensitive(m_action_name, true);
	gtk_widget_set_sensitive(m_action_pattern, true);
	gtk_widget_set_sensitive(m_action_command, true);
	gtk_widget_set_sensitive(m_action_regex, true);
}

//-----------------------------------------------------------------------------

void SettingsDialog::remove_action(GtkButton* button)
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
		path = nullptr;
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
		gtk_tree_view_set_cursor(m_actions_view, path, nullptr, false);
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

void SettingsDialog::response(GtkDialog*, int response_id)
{
	if (response_id == GTK_RESPONSE_HELP)
	{
		bool result = g_spawn_command_line_async("exo-open --launch WebBrowser " PLUGIN_WEBSITE, nullptr);

		if (G_UNLIKELY(!result))
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

		for (auto command : wm_settings->command)
		{
			command->check();
		}

		if (response_id == GTK_RESPONSE_CLOSE)
		{
			gtk_widget_destroy(m_window);
		}
	}
}

//-----------------------------------------------------------------------------

GtkWidget* SettingsDialog::init_general_tab()
{
	// Create general page
	GtkGrid* page = GTK_GRID(gtk_grid_new());
	gtk_container_set_border_width(GTK_CONTAINER(page), 12);
	gtk_grid_set_column_spacing(page, 12);
	gtk_grid_set_row_spacing(page, 6);


	// Create box to display view layout
	GtkButtonBox* display_box = GTK_BUTTON_BOX(gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL));
	gtk_widget_set_halign(GTK_WIDGET(display_box), GTK_ALIGN_CENTER);
	gtk_widget_set_hexpand(GTK_WIDGET(display_box), false);
	gtk_button_box_set_layout(display_box, GTK_BUTTONBOX_EXPAND);
	gtk_grid_attach(page, GTK_WIDGET(display_box), 0, 0, 2, 1);

	// Add option to show as icons
	m_show_as_icons = gtk_radio_button_new_with_mnemonic(nullptr, _("Show as _icons"));
	{
		const gchar* icons[] = {
			"view-list-icons",
			"view-grid",
			nullptr
		};
		GIcon* gicon = g_themed_icon_new_from_names(const_cast<gchar**>(icons), -1);
		gtk_button_set_image(GTK_BUTTON(m_show_as_icons), gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_DND));
		g_object_unref(gicon);
	}
	gtk_button_set_image_position(GTK_BUTTON(m_show_as_icons), GTK_POS_TOP);
	gtk_button_set_always_show_image(GTK_BUTTON(m_show_as_icons), true);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(m_show_as_icons), false);
	gtk_box_pack_start(GTK_BOX(display_box), m_show_as_icons, true, true, 0);

	// Add option to show as list
	m_show_as_list = gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(m_show_as_icons), _("Show as lis_t"));
	{
		const gchar* icons[] = {
			"view-list-compact",
			"view-list-details",
			"view-list",
			nullptr
		};
		GIcon* gicon = g_themed_icon_new_from_names(const_cast<gchar**>(icons), -1);
		gtk_button_set_image(GTK_BUTTON(m_show_as_list), gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_DND));
		g_object_unref(gicon);
	}
	gtk_button_set_image_position(GTK_BUTTON(m_show_as_list), GTK_POS_TOP);
	gtk_button_set_always_show_image(GTK_BUTTON(m_show_as_list), true);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(m_show_as_list), false);
	gtk_box_pack_start(GTK_BOX(display_box), m_show_as_list, true, true, 0);

	// Add option to show as tree
	m_show_as_tree = gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(m_show_as_list), _("Show as t_ree"));
	{
		const gchar* icons[] = {
			"view-list-tree",
			"view-list-details",
			"pan-end",
			nullptr
		};
		GIcon* gicon = g_themed_icon_new_from_names(const_cast<gchar**>(icons), -1);
		gtk_button_set_image(GTK_BUTTON(m_show_as_tree), gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_DND));
		g_object_unref(gicon);
	}
	gtk_button_set_image_position(GTK_BUTTON(m_show_as_tree), GTK_POS_TOP);
	gtk_button_set_always_show_image(GTK_BUTTON(m_show_as_tree), true);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(m_show_as_tree), false);
	gtk_box_pack_start(GTK_BOX(display_box), m_show_as_tree, true, true, 0);

	if (wm_settings->view_as_icons)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_as_icons), true);
	}
	else if (wm_settings->load_hierarchy)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_as_tree), true);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_as_list), true);
	}
	g_signal_connect_slot(m_show_as_icons, "toggled", &SettingsDialog::toggle_show_as_icons, this);
	g_signal_connect_slot(m_show_as_list, "toggled", &SettingsDialog::toggle_show_as_list, this);
	g_signal_connect_slot(m_show_as_tree, "toggled", &SettingsDialog::toggle_show_as_tree, this);

	// Add space beneath options
	gtk_widget_set_margin_bottom(GTK_WIDGET(display_box), 12);


	// Add option to use generic names
	m_show_generic_names = gtk_check_button_new_with_mnemonic(_("Show generic application _names"));
	gtk_grid_attach(page, m_show_generic_names, 0, 1, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_generic_names), !wm_settings->launcher_show_name);
	g_signal_connect_slot(m_show_generic_names, "toggled", &SettingsDialog::toggle_show_generic_name, this);

	// Add option to hide category names
	m_show_category_names = gtk_check_button_new_with_mnemonic(_("Show cate_gory names"));
	gtk_grid_attach(page, m_show_category_names, 0, 2, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_category_names), wm_settings->category_show_name);
	gtk_widget_set_sensitive(m_show_category_names, (wm_settings->category_icon_size != -1) && !wm_settings->position_categories_horizontal);
	g_signal_connect_slot(m_show_category_names, "toggled", &SettingsDialog::toggle_show_category_name, this);

	// Add option to hide tooltips
	m_show_tooltips = gtk_check_button_new_with_mnemonic(_("Show application too_ltips"));
	gtk_grid_attach(page, m_show_tooltips, 0, 3, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_tooltips), wm_settings->launcher_show_tooltip);
	g_signal_connect_slot(m_show_tooltips, "toggled", &SettingsDialog::toggle_show_tooltip, this);

	// Add option to hide descriptions
	m_show_descriptions = gtk_check_button_new_with_mnemonic(_("Show application _descriptions"));
	gtk_grid_attach(page, m_show_descriptions, 0, 4, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_descriptions), wm_settings->launcher_show_description);
	gtk_widget_set_sensitive(m_show_descriptions, !wm_settings->view_as_icons);
	g_signal_connect_slot(m_show_descriptions, "toggled", &SettingsDialog::toggle_show_description, this);

	// Add space beneath options
	gtk_widget_set_margin_bottom(m_show_descriptions, 12);


	// Add item icon size selector
	GtkWidget* label = gtk_label_new_with_mnemonic(_("Application icon si_ze:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(page, label, 0, 5, 1, 1);

	m_item_icon_size = gtk_combo_box_text_new();
	gtk_widget_set_halign(m_item_icon_size, GTK_ALIGN_START);
	gtk_widget_set_hexpand(m_item_icon_size, false);
	const auto icon_sizes = IconSize::get_strings();
	for (const auto& icon_size : icon_sizes)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_item_icon_size), icon_size.c_str());
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_item_icon_size), wm_settings->launcher_icon_size + 1);
	gtk_grid_attach(page, m_item_icon_size, 1, 5, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_item_icon_size);
	g_signal_connect_slot(m_item_icon_size, "changed", &SettingsDialog::item_icon_size_changed, this);

	// Add category icon size selector
	label = gtk_label_new_with_mnemonic(_("Categ_ory icon size:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(page, label, 0, 6, 1, 1);

	m_category_icon_size = gtk_combo_box_text_new();
	gtk_widget_set_halign(m_category_icon_size, GTK_ALIGN_START);
	gtk_widget_set_hexpand(m_category_icon_size, false);
	for (const auto& icon_size : icon_sizes)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_category_icon_size), icon_size.c_str());
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_category_icon_size), wm_settings->category_icon_size + 1);
	gtk_grid_attach(page, m_category_icon_size, 1, 6, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_category_icon_size);
	g_signal_connect_slot(m_category_icon_size, "changed", &SettingsDialog::category_icon_size_changed, this);

	// Add space beneath options
	gtk_widget_set_margin_bottom(label, 12);
	gtk_widget_set_margin_bottom(m_category_icon_size, 12);


	// Add option to control background opacity
	label = gtk_label_new_with_mnemonic(_("Background opacit_y:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(page, label, 0, 7, 1, 1);

	m_background_opacity = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
	gtk_widget_set_hexpand(GTK_WIDGET(m_background_opacity), true);
	gtk_grid_attach(page, m_background_opacity, 1, 7, 1, 1);
	gtk_scale_set_value_pos(GTK_SCALE(m_background_opacity), GTK_POS_RIGHT);
	gtk_range_set_value(GTK_RANGE(m_background_opacity), wm_settings->menu_opacity);
	g_signal_connect_slot(m_background_opacity, "value-changed", &SettingsDialog::background_opacity_changed, this);

	GdkScreen* screen = gtk_widget_get_screen(m_window);
	const bool enabled = gdk_screen_is_composited(screen);
	gtk_widget_set_sensitive(label, enabled);
	gtk_widget_set_sensitive(GTK_WIDGET(m_background_opacity), enabled);

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------

GtkWidget* SettingsDialog::init_appearance_tab()
{
	// Create appearance page
	GtkBox* page = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 18));
	gtk_container_set_border_width(GTK_CONTAINER(page), 12);


	// Align labels across sections
	GtkSizeGroup* label_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	GtkSizeGroup* size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);


	// Create menu section
	GtkGrid* menu_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(menu_table, 12);
	gtk_grid_set_row_spacing(menu_table, 6);

	GtkWidget* behavior_frame = make_aligned_frame(_("Menu"), GTK_WIDGET(menu_table));
	gtk_box_pack_start(page, behavior_frame, false, false, 0);

	// Add option to use horizontal categories
	m_position_categories_horizontal = gtk_check_button_new_with_mnemonic(_("Position categories _horizontally"));
	gtk_grid_attach(menu_table, m_position_categories_horizontal, 0, 0, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_categories_horizontal), wm_settings->position_categories_horizontal);
	g_signal_connect_slot(m_position_categories_horizontal, "toggled", &SettingsDialog::toggle_position_categories_horizontal, this);

	// Add option to use alternate categories position
	m_position_categories_alternate = gtk_check_button_new_with_mnemonic(_("Position cate_gories next to panel button"));
	gtk_grid_attach(menu_table, m_position_categories_alternate, 0, 1, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_categories_alternate), wm_settings->position_categories_alternate);
	g_signal_connect_slot(m_position_categories_alternate, "toggled", &SettingsDialog::toggle_position_categories_alternate, this);

	// Add option to use alternate search entry position
	m_position_search_alternate = gtk_check_button_new_with_mnemonic(_("Position _search entry next to panel button"));
	gtk_grid_attach(menu_table, m_position_search_alternate, 0, 2, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_search_alternate), wm_settings->position_search_alternate);
	g_signal_connect_slot(m_position_search_alternate, "toggled", &SettingsDialog::toggle_position_search_alternate, this);

	// Add option to use alternate commands position
	m_position_commands_alternate = gtk_check_button_new_with_mnemonic(_("Position commands next to search _entry"));
	gtk_grid_attach(menu_table, m_position_commands_alternate, 0, 3, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_position_commands_alternate), wm_settings->position_commands_alternate);
	g_signal_connect_slot(m_position_commands_alternate, "toggled", &SettingsDialog::toggle_position_commands_alternate, this);


	// Add profile shape selector
	GtkWidget* label = gtk_label_new_with_mnemonic(_("P_rofile:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(menu_table, label, 0, 4, 1, 1);

	m_profile_shape = gtk_combo_box_text_new();
	gtk_widget_set_halign(m_profile_shape, GTK_ALIGN_START);
	gtk_widget_set_hexpand(m_profile_shape, true);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_profile_shape), _("Round Picture"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_profile_shape), _("Square Picture"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_profile_shape), _("Hidden"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_profile_shape), wm_settings->profile_shape);
	gtk_grid_attach(menu_table, m_profile_shape, 1, 4, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_profile_shape);
	g_signal_connect_slot(m_profile_shape, "changed", &SettingsDialog::profile_shape_changed, this);

	gtk_size_group_add_widget(label_size_group, label);
	gtk_size_group_add_widget(size_group, m_profile_shape);


	// Create panel button section
	GtkGrid* panel_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(panel_table, 12);
	gtk_grid_set_row_spacing(panel_table, 6);

	GtkWidget* recent_frame = make_aligned_frame(_("Panel Button"), GTK_WIDGET(panel_table));
	gtk_box_pack_start(page, recent_frame, false, false, 0);

	// Add button style selector
	label = gtk_label_new_with_mnemonic(_("Di_splay:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(panel_table, label, 0, 0, 1, 1);

	m_button_style = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Icon"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Title"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Icon and title"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_button_style), static_cast<int>(m_plugin->get_button_style()) - 1);
	gtk_widget_set_halign(m_button_style, GTK_ALIGN_START);
	gtk_widget_set_hexpand(m_button_style, false);
	gtk_grid_attach(panel_table, m_button_style, 1, 0, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_button_style);
	g_signal_connect_slot(m_button_style, "changed", &SettingsDialog::style_changed, this);

	gtk_size_group_add_widget(label_size_group, label);
	gtk_size_group_add_widget(size_group, m_button_style);

	// Add title selector
	label = gtk_label_new_with_mnemonic(_("_Title:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(panel_table, label, 0, 1, 1, 1);

	m_title = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_title), m_plugin->get_button_title().c_str());
	gtk_widget_set_hexpand(m_title, true);
	gtk_grid_attach(panel_table, m_title, 1, 1, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_title);
	g_signal_connect_slot(m_title, "changed", &SettingsDialog::title_changed, this);

	// Add icon selector
	label = gtk_label_new_with_mnemonic(_("_Icon:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(panel_table, label, 0, 2, 1, 1);

	m_icon_button = gtk_button_new();
	gtk_widget_set_halign(m_icon_button, GTK_ALIGN_START);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_icon_button);
	g_signal_connect_slot<GtkButton*>(m_icon_button, "clicked", &SettingsDialog::choose_icon, this);
	gtk_grid_attach(panel_table, m_icon_button, 1, 2, 1, 1);

	m_icon = gtk_image_new_from_icon_name(m_plugin->get_button_icon_name().c_str(), GTK_ICON_SIZE_DIALOG);
	gtk_container_add(GTK_CONTAINER(m_icon_button), m_icon);

	m_button_single_row = gtk_check_button_new_with_mnemonic(_("Use a single _panel row"));
	gtk_grid_attach(panel_table, m_button_single_row, 1, 3, 1, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button_single_row), wm_settings->button_single_row);
	gtk_widget_set_sensitive(m_button_single_row, gtk_combo_box_get_active(GTK_COMBO_BOX (m_button_style)) == 0);
	g_signal_connect_slot(m_button_single_row, "toggled", &SettingsDialog::toggle_button_single_row, this);
	gtk_widget_show(m_button_single_row);

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------

GtkWidget* SettingsDialog::init_behavior_tab()
{
	// Create behavior page
	GtkBox* page = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 18));
	gtk_container_set_border_width(GTK_CONTAINER(page), 12);


	// Create default display section
	GtkBox* display_vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	GtkWidget* display_frame = make_aligned_frame(_("Default Category"), GTK_WIDGET(display_vbox));
	gtk_box_pack_start(page, display_frame, false, false, 0);

	// Add option to display favorites
	m_display_favorites = gtk_radio_button_new_with_mnemonic(nullptr, _("Favorites"));
	gtk_box_pack_start(display_vbox, m_display_favorites, true, true, 0);

	// Add option to display recently used
	m_display_recent = gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(m_display_favorites), _("Recently Used"));
	gtk_box_pack_start(display_vbox, m_display_recent, true, true, 0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_display_recent), wm_settings->recent_items_max);

	// Add option to display all applications
	m_display_applications = gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(m_display_recent), _("All Applications"));
	gtk_box_pack_start(display_vbox, m_display_applications, true, true, 0);

	switch (wm_settings->default_category)
	{
	case 1:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_display_recent), true);
		break;

	case 2:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_display_applications), true);
		break;

	default:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_display_favorites), true);
		break;
	}
	g_signal_connect_slot(m_display_favorites, "toggled", &SettingsDialog::toggle_display_favorites, this);
	g_signal_connect_slot(m_display_recent, "toggled", &SettingsDialog::toggle_display_recent, this);
	g_signal_connect_slot(m_display_applications, "toggled", &SettingsDialog::toggle_display_applications, this);


	// Create menu section
	GtkBox* behavior_vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	GtkWidget* behavior_frame = make_aligned_frame(_("Menu"), GTK_WIDGET(behavior_vbox));
	gtk_box_pack_start(page, behavior_frame, false, false, 0);

	// Add option to switch categories by hovering
	m_hover_switch_category = gtk_check_button_new_with_mnemonic(_("Switch categories by _hovering"));
	gtk_box_pack_start(behavior_vbox, m_hover_switch_category, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_hover_switch_category), wm_settings->category_hover_activate);
	g_signal_connect_slot(m_hover_switch_category, "toggled", &SettingsDialog::toggle_hover_switch_category, this);

	// Add option to stay when menu loses focus
	m_stay_on_focus_out = gtk_check_button_new_with_mnemonic(_("Stay _visible when focus is lost"));
	gtk_box_pack_start(behavior_vbox, m_stay_on_focus_out, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_stay_on_focus_out), wm_settings->stay_on_focus_out);
	g_signal_connect_slot(m_stay_on_focus_out, "toggled", &SettingsDialog::toggle_stay_on_focus_out, this);

	// Add option to sort categories
	m_sort_categories = gtk_check_button_new_with_mnemonic(_("Sort ca_tegories"));
	gtk_box_pack_start(behavior_vbox, m_sort_categories, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_sort_categories), wm_settings->sort_categories);
	g_signal_connect_slot(m_sort_categories, "toggled", &SettingsDialog::toggle_sort_categories, this);


	// Create recently used section
	GtkGrid* recent_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(recent_table, 12);
	gtk_grid_set_row_spacing(recent_table, 6);

	GtkWidget* recent_frame = make_aligned_frame(_("Recently Used"), GTK_WIDGET(recent_table));
	gtk_box_pack_start(page, recent_frame, false, false, 0);

	// Add value to change maximum number of recently used entries
	GtkWidget* label = gtk_label_new_with_mnemonic(_("Amount of _items:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(recent_table, label, 0, 0, 1, 1);

	m_recent_items_max = gtk_spin_button_new_with_range(0, 100, 1);
	gtk_grid_attach(recent_table, m_recent_items_max, 1, 0, 1, 1);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_recent_items_max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_recent_items_max), wm_settings->recent_items_max);
	g_signal_connect_slot(m_recent_items_max, "value-changed", &SettingsDialog::recent_items_max_changed, this);

	// Add option to remember favorites
	m_remember_favorites = gtk_check_button_new_with_mnemonic(_("Include _favorites"));
	gtk_grid_attach(recent_table, m_remember_favorites, 0, 1, 2, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_remember_favorites), wm_settings->favorites_in_recent);
	g_signal_connect_slot(m_remember_favorites, "toggled", &SettingsDialog::toggle_remember_favorites, this);


	// Create command buttons section
	GtkBox* command_vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	GtkWidget* command_frame = make_aligned_frame(_("Session Commands"), GTK_WIDGET(command_vbox));
	gtk_box_pack_start(page, command_frame, false, false, 0);

	// Add option to show confirmation dialogs
	m_confirm_session_command = gtk_check_button_new_with_mnemonic(_("Show c_onfirmation dialog"));
	gtk_box_pack_start(command_vbox, m_confirm_session_command, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_confirm_session_command), wm_settings->confirm_session_command);
	g_signal_connect_slot(m_confirm_session_command, "toggled", &SettingsDialog::toggle_confirm_session_command, this);

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------

GtkWidget* SettingsDialog::init_commands_tab()
{
	// Create commands page
	GtkBox* page = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	gtk_container_set_border_width(GTK_CONTAINER(page), 12);
	GtkSizeGroup* label_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	// Add command entries
	for (auto command : wm_settings->command)
	{
		CommandEdit* command_edit = new CommandEdit(command, label_size_group);
		gtk_box_pack_start(page, command_edit->get_widget(), false, false, 0);
		m_commands.push_back(command_edit);
	}

	return GTK_WIDGET(page);
}

//-----------------------------------------------------------------------------

GtkWidget* SettingsDialog::init_search_actions_tab()
{
	// Create search actions page
	GtkGrid* page = GTK_GRID(gtk_grid_new());
	gtk_container_set_border_width(GTK_CONTAINER(page), 12);
	gtk_grid_set_column_spacing(page, 6);
	gtk_grid_set_row_spacing(page, 6);

	// Create model
	m_actions_model = gtk_list_store_new(N_COLUMNS,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_POINTER);
	for (auto action : wm_settings->search_actions)
	{
		gtk_list_store_insert_with_values(m_actions_model,
				nullptr, G_MAXINT,
				COLUMN_NAME, action->get_name(),
				COLUMN_PATTERN, action->get_pattern(),
				COLUMN_ACTION, action,
				-1);
	}

	// Create view
	m_actions_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_actions_model)));
	g_signal_connect_slot(m_actions_view, "cursor-changed", &SettingsDialog::action_selected, this);

	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Name"),
			renderer, "text", COLUMN_NAME, nullptr);
	gtk_tree_view_append_column(m_actions_view, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Pattern"),
			renderer, "text", COLUMN_PATTERN, nullptr);
	gtk_tree_view_append_column(m_actions_view, column);

	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_actions_view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);

	GtkWidget* scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
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
	g_signal_connect_slot(m_action_add, "clicked", &SettingsDialog::add_action, this);

	m_action_remove = gtk_button_new();
	gtk_widget_set_tooltip_text(m_action_remove, _("Remove selected action"));
	gtk_widget_show(m_action_remove);

	image = gtk_image_new_from_icon_name("list-remove", GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(m_action_remove), image);
	gtk_widget_show(image);
	g_signal_connect_slot(m_action_remove, "clicked", &SettingsDialog::remove_action, this);

	GtkBox* actions_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
	gtk_widget_set_halign(GTK_WIDGET(actions_box), GTK_ALIGN_START);
	gtk_box_pack_start(actions_box, m_action_add, false, false, 0);
	gtk_box_pack_start(actions_box, m_action_remove, false, false, 0);
	gtk_grid_attach(page, GTK_WIDGET(actions_box), 1, 0, 1, 1);
	gtk_widget_show_all(GTK_WIDGET(actions_box));

	// Create details section
	GtkGrid* details_table = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(details_table, 12);
	gtk_grid_set_row_spacing(details_table, 6);
	GtkWidget* details_frame = make_aligned_frame(_("Details"), GTK_WIDGET(details_table));
	gtk_grid_attach(page, details_frame, 0, 1, 2, 1);

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
	g_signal_connect_slot(m_action_name, "changed", &SettingsDialog::action_name_changed, this);

	// Create entry for keyword
	label = gtk_label_new_with_mnemonic(_("_Pattern:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_widget_show(label);
	gtk_grid_attach(details_table, label, 0, 1, 1, 1);

	m_action_pattern = gtk_entry_new();
	gtk_widget_show(m_action_pattern);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_action_pattern);
	gtk_grid_attach(details_table, m_action_pattern, 1, 1, 1, 1);
	g_signal_connect_slot(m_action_pattern, "changed", &SettingsDialog::action_pattern_changed, this);

	// Create entry for command
	label = gtk_label_new_with_mnemonic(_("C_ommand:"));
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_widget_show(label);
	gtk_grid_attach(details_table, label, 0, 2, 1, 1);

	m_action_command = gtk_entry_new();
	gtk_widget_show(m_action_command);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_action_command);
	gtk_grid_attach(details_table, m_action_command, 1, 2, 1, 1);
	g_signal_connect_slot(m_action_command, "changed", &SettingsDialog::action_command_changed, this);

	// Create toggle button for regular expressions
	m_action_regex = gtk_check_button_new_with_mnemonic(_("_Regular expression"));
	gtk_widget_show(m_action_regex);
	gtk_grid_attach(details_table, m_action_regex, 1, 3, 1, 1);
	g_signal_connect_slot(m_action_regex, "toggled", &SettingsDialog::action_toggle_regex, this);

	// Select first action
	if (!wm_settings->search_actions.empty())
	{
		GtkTreePath* path = gtk_tree_path_new_first();
		gtk_tree_view_set_cursor(m_actions_view, path, nullptr, false);
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
