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


#ifndef WHISKERMENU_SECTION_BUTTON_HPP
#define WHISKERMENU_SECTION_BUTTON_HPP

extern "C"
{
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
}

namespace WhiskerMenu
{

class SectionButton
{
public:
	SectionButton(const gchar* icon, const gchar* text);
	~SectionButton();

	GtkRadioButton* get_button() const
	{
		return m_button;
	}

	bool get_active() const
	{
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_button));
	}

	void set_active(bool active)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button), active);
	}

	GSList* get_group() const
	{
		return gtk_radio_button_get_group(m_button);
	}

	void set_group(GSList* group)
	{
		gtk_radio_button_set_group(m_button, group);
	}

	void reload_icon_size();

	static bool get_hover_activate();
	static void set_hover_activate(bool hover_activate);
	static int get_icon_size();
	static void set_icon_size(const int size);

private:
	GtkRadioButton* m_button;
	XfcePanelImage* m_icon;
};

}

#endif // WHISKERMENU_SECTION_BUTTON_HPP
