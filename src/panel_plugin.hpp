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
	void set_button_icon_name(std::string icon);
	void set_configure_enabled(bool enabled);

private:
	gboolean button_clicked(GtkWidget*, GdkEventButton* event);
	void menu_hidden();
	void menu_shown();
	void configure();
	gboolean remote_event(XfcePanelPlugin* plugin, gchar* name, GValue* value);
	void save();
	gboolean size_changed(XfcePanelPlugin*, gint size);

private:
	XfcePanelPlugin* m_plugin;
	GtkWidget* m_button;
	std::string m_button_icon_name;
	XfcePanelImage* m_button_icon;
	Menu* m_menu;
};

}

#endif // WHISKERMENU_PANEL_PLUGIN_HPP
