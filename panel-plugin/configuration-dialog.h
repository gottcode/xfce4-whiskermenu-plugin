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
	void category_icon_size_changed(GtkComboBox* combo);
	void item_icon_size_changed(GtkComboBox* combo);

	void style_changed(GtkComboBox* combo);
	void title_changed(GtkEditable* editable);
	void choose_icon();

	void toggle_hover_switch_category(GtkToggleButton* button);
	void toggle_remember_favorites(GtkToggleButton* button);
	void toggle_display_recent(GtkToggleButton* button);

	void response(GtkDialog*, int response_id);
	GtkWidget* init_appearance_tab();
	GtkWidget* init_behavior_tab();

private:
	Plugin* m_plugin;
	GtkWidget* m_window;

	GtkWidget* m_show_generic_names;
	GtkWidget* m_show_descriptions;
	GtkWidget* m_show_hierarchy;
	GtkWidget* m_position_search_alternate;
	GtkWidget* m_position_commands_alternate;
	GtkWidget* m_category_icon_size;
	GtkWidget* m_item_icon_size;

	GtkWidget* m_button_style;
	GtkWidget* m_title;
	GtkWidget* m_icon;
	GtkWidget* m_icon_button;

	GtkWidget* m_hover_switch_category;
	GtkWidget* m_remember_favorites;
	GtkWidget* m_display_recent;
	std::vector<CommandEdit*> m_commands;
};

}

#endif // WHISKERMENU_CONFIGURATION_DIALOG_H
