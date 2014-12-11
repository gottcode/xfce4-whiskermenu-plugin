/*
 * Copyright (C) 2013, 2014 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_PLUGIN_H
#define WHISKERMENU_PLUGIN_H

#include <string>

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>

namespace WhiskerMenu
{

class Window;

class Plugin
{
public:
	explicit Plugin(XfcePanelPlugin* plugin);
	~Plugin();

	GtkWidget* get_button() const
	{
		return m_button;
	}

	enum ButtonStyle
	{
		ShowIcon = 0x1,
		ShowText = 0x2,
		ShowIconAndText = ShowIcon | ShowText
	};

	ButtonStyle get_button_style() const;
	std::string get_button_title() const;
	static std::string get_button_title_default();
	std::string get_button_icon_name() const;

	void reload();
	void set_button_style(ButtonStyle style);
	void set_button_title(const std::string& title);
	void set_button_icon_name(const std::string& icon);
	void set_configure_enabled(bool enabled);

private:
	void button_toggled(GtkToggleButton* button);
	void menu_hidden();
	void configure();
#if (LIBXFCE4PANEL_CHECK_VERSION(4,9,0))
	void mode_changed(XfcePanelPlugin*, XfcePanelPluginMode mode);
#else
	void orientation_changed(XfcePanelPlugin*, GtkOrientation orientation);
#endif
	gboolean remote_event(XfcePanelPlugin*, gchar* name, GValue* value);
	void save();
	void show_about();
	gboolean size_changed(XfcePanelPlugin*, gint size);

private:
	XfcePanelPlugin* m_plugin;
	Window* m_window;

	GtkWidget* m_button;
	GtkBox* m_button_box;
	GtkLabel* m_button_label;
	XfcePanelImage* m_button_icon;
};

}

#endif // WHISKERMENU_PLUGIN_H
