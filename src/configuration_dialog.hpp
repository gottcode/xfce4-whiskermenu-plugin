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


#ifndef WHISKERMENU_CONFIGURATION_DIALOG_HPP
#define WHISKERMENU_CONFIGURATION_DIALOG_HPP

#include "slot.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

namespace WhiskerMenu
{

class PanelPlugin;

class ConfigurationDialog
{
public:
	explicit ConfigurationDialog(PanelPlugin* plugin);
	~ConfigurationDialog();

	GtkWidget* get_widget() const
	{
		return m_window;
	}

private:
	SLOT_0(void, ConfigurationDialog, choose_icon);
	SLOT_1(void, ConfigurationDialog, toggle_hover_switch_category, GtkToggleButton*);
	SLOT_1(void, ConfigurationDialog, toggle_show_name, GtkToggleButton*);
	SLOT_1(void, ConfigurationDialog, toggle_show_description, GtkToggleButton*);
	SLOT_2(void, ConfigurationDialog, response, GtkDialog*, gint);

private:
	PanelPlugin* m_plugin;

	GtkWidget* m_window;
	GtkWidget* m_icon;
	GtkWidget* m_icon_button;
	GtkWidget* m_show_names;
	GtkWidget* m_show_descriptions;
	GtkWidget* m_hover_switch_category;
};

}

#endif // WHISKERMENU_CONFIGURATION_DIALOG_HPP
