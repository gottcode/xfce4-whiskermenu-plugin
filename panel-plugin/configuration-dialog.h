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

#ifndef WHISKERMENU_CONFIGURATION_DIALOG_H
#define WHISKERMENU_CONFIGURATION_DIALOG_H

#include <vector>

#include <gtk/gtk.h>

namespace WhiskerMenu
{

class CommandEdit;
class Plugin;
class SearchAction;

class ConfigurationDialog
{
public:
	explicit ConfigurationDialog(Plugin* plugin);
	~ConfigurationDialog();

	GtkWidget* get_widget() const
	{
		return m_window;
	}

private:
	void toggle_show_generic_name(GtkToggleButton* button);
	void toggle_show_description(GtkToggleButton* button);
	void toggle_show_hierarchy(GtkToggleButton* button);
	void toggle_position_search_alternate(GtkToggleButton* button);
	void toggle_position_commands_alternate(GtkToggleButton* button);
	void toggle_position_categories_alternate(GtkToggleButton* button);
	void category_icon_size_changed(GtkComboBox* combo);
	void item_icon_size_changed(GtkComboBox* combo);

	void style_changed(GtkComboBox* combo);
	void title_changed(GtkEditable* editable);
	void choose_icon();

	void toggle_button_single_row(GtkToggleButton* button);
	void toggle_hover_switch_category(GtkToggleButton* button);

	void recent_items_max_changed(GtkSpinButton* button);
	void toggle_remember_favorites(GtkToggleButton* button);
	void toggle_display_recent(GtkToggleButton* button);

	SearchAction* get_selected_action(GtkTreeIter* iter = NULL) const;
	void action_selected(GtkTreeView* view);
	void action_name_changed(GtkEditable* editable);
	void action_pattern_changed(GtkEditable* editable);
	void action_command_changed(GtkEditable* editable);
	void action_toggle_regex(GtkToggleButton* button);
	void add_action(GtkButton*);
	void remove_action(GtkButton* button);

	void response(GtkDialog*, int response_id);
	GtkWidget* init_appearance_tab();
	GtkWidget* init_behavior_tab();
	GtkWidget* init_commands_tab();
	GtkWidget* init_search_actions_tab();

private:
	Plugin* m_plugin;
	GtkWidget* m_window;

	GtkWidget* m_button_single_row;
	GtkWidget* m_show_generic_names;
	GtkWidget* m_show_descriptions;
	GtkWidget* m_show_hierarchy;
	GtkWidget* m_position_search_alternate;
	GtkWidget* m_position_commands_alternate;
	GtkWidget* m_position_categories_alternate;
	GtkWidget* m_category_icon_size;
	GtkWidget* m_item_icon_size;

	GtkWidget* m_button_style;
	GtkWidget* m_title;
	GtkWidget* m_icon;
	GtkWidget* m_icon_button;

	GtkWidget* m_hover_switch_category;
	GtkWidget* m_remember_favorites;
	GtkWidget* m_display_recent;
	GtkWidget* m_recent_items_max;
	std::vector<CommandEdit*> m_commands;

	GtkTreeView* m_actions_view;
	GtkListStore* m_actions_model;
	GtkWidget* m_action_add;
	GtkWidget* m_action_remove;
	GtkWidget* m_action_name;
	GtkWidget* m_action_pattern;
	GtkWidget* m_action_command;
	GtkWidget* m_action_regex;
};

}

#endif // WHISKERMENU_CONFIGURATION_DIALOG_H
