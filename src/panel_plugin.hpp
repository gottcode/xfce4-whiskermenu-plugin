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


#ifndef WHISKERMENU_PANEL_PLUGIN_HPP
#define WHISKERMENU_PANEL_PLUGIN_HPP

#include "slot.hpp"

#include <string>

extern "C"
{
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
}

namespace WhiskerMenu
{

class Menu;

class PanelPlugin
{
public:
	PanelPlugin(XfcePanelPlugin* plugin);
	~PanelPlugin();

	GtkWidget* get_button() const
	{
		return m_button;
	}

	std::string get_button_icon_name() const
	{
		return m_button_icon_name;
	}

	void reload();
	void set_button_icon_name(const std::string& icon);
	void set_configure_enabled(bool enabled);

private:
	SLOT_2(gboolean, PanelPlugin, button_clicked, GtkWidget*, GdkEventButton*);
	SLOT_0(void, PanelPlugin, menu_hidden);
	SLOT_0(void, PanelPlugin, configure);
	SLOT_3(gboolean, PanelPlugin, remote_event, XfcePanelPlugin*, gchar*, GValue*);
	SLOT_0(void, PanelPlugin, save);
	SLOT_2(gboolean, PanelPlugin, size_changed, XfcePanelPlugin*, gint);

private:
	void popup_menu();

private:
	XfcePanelPlugin* m_plugin;
	GtkWidget* m_button;
	std::string m_button_icon_name;
	XfcePanelImage* m_button_icon;
	Menu* m_menu;
};

}

#endif // WHISKERMENU_PANEL_PLUGIN_HPP
