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


#include "section_button.hpp"

#include "icon_size.hpp"

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static bool f_hover_activate = false;
static WhiskerMenu::IconSize f_icon_size(WhiskerMenu::IconSize::Smaller);

static gboolean hover_timeout(GtkToggleButton* button)
{
	if (gtk_widget_get_state(GTK_WIDGET(button)) == GTK_STATE_PRELIGHT)
	{
		gtk_toggle_button_set_active(button, true);
	}
	return false;
}

static gboolean on_enter_notify_event(GtkWidget*, GdkEventCrossing*, GtkToggleButton* button)
{
	if (f_hover_activate && !gtk_toggle_button_get_active(button))
	{
		g_timeout_add(150, (GSourceFunc)hover_timeout, button);
	}
	return false;
}

//-----------------------------------------------------------------------------

SectionButton::SectionButton(const gchar* icon, const gchar* text)
{
	m_button = GTK_RADIO_BUTTON(gtk_radio_button_new(NULL));
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(m_button), false);
	gtk_button_set_relief(GTK_BUTTON(m_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(m_button), false);
	g_signal_connect(m_button, "enter-notify-event", G_CALLBACK(on_enter_notify_event), GTK_TOGGLE_BUTTON(m_button));

	GtkBox* box = GTK_BOX(gtk_hbox_new(false, 4));
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(box));

	m_icon = XFCE_PANEL_IMAGE(xfce_panel_image_new_from_source(icon));
	reload_icon_size();
	gtk_box_pack_start(box, GTK_WIDGET(m_icon), false, false, 0);

	GtkWidget* label = gtk_label_new(text);
	gtk_box_pack_start(box, label, false, true, 0);
}

//-----------------------------------------------------------------------------

SectionButton::~SectionButton()
{
	gtk_widget_destroy(GTK_WIDGET(m_button));
}

//-----------------------------------------------------------------------------

void SectionButton::reload_icon_size()
{
	xfce_panel_image_set_size(m_icon, f_icon_size.get_size());
}

//-----------------------------------------------------------------------------

bool SectionButton::get_hover_activate()
{
	return f_hover_activate;
}

//-----------------------------------------------------------------------------

void SectionButton::set_hover_activate(bool hover_activate)
{
	f_hover_activate = hover_activate;
}

//-----------------------------------------------------------------------------

int SectionButton::get_icon_size()
{
	return f_icon_size;
}

//-----------------------------------------------------------------------------

void SectionButton::set_icon_size(const int size)
{
	f_icon_size = size;
}

//-----------------------------------------------------------------------------
