/*
 * Copyright (C) 2013, 2016, 2017 Graeme Gott <graeme@gottcode.org>
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

#include "section-button.h"

#include "settings.h"

#include <libxfce4panel/libxfce4panel.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static gboolean hover_timeout(GtkToggleButton* button)
{
	if (gtk_widget_get_state_flags(GTK_WIDGET(button)) & GTK_STATE_FLAG_PRELIGHT)
	{
		gtk_toggle_button_set_active(button, true);
	}
	return false;
}

static gboolean on_enter_notify_event(GtkWidget*, GdkEventCrossing*, GtkToggleButton* button)
{
	if (wm_settings->category_hover_activate && !gtk_toggle_button_get_active(button))
	{
		g_timeout_add(150, (GSourceFunc)hover_timeout, button);
	}
	return false;
}

//-----------------------------------------------------------------------------

SectionButton::SectionButton(const gchar* icon, const gchar* text) :
	m_icon_name(g_strdup(icon))
{
	m_button = GTK_RADIO_BUTTON(gtk_radio_button_new(NULL));
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(m_button), false);
	gtk_button_set_relief(GTK_BUTTON(m_button), GTK_RELIEF_NONE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(m_button), text);
#if GTK_CHECK_VERSION(3,20,0)
	gtk_widget_set_focus_on_click(GTK_WIDGET(m_button), false);
#else
	gtk_button_set_focus_on_click(GTK_BUTTON(m_button), false);
#endif
	g_signal_connect(m_button, "enter-notify-event", G_CALLBACK(on_enter_notify_event), GTK_TOGGLE_BUTTON(m_button));

	m_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(m_box));

	if (!g_path_is_absolute(icon))
	{
		m_icon = gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_BUTTON);
	}
	else
	{
		GFile* file = g_file_new_for_path(icon);
		GIcon* gicon = g_file_icon_new(file);
		m_icon = gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_BUTTON);
		g_object_unref(gicon);
		g_object_unref(file);
	}
	gtk_box_pack_start(m_box, m_icon, false, false, 0);

	m_label = gtk_label_new(text);
	gtk_box_pack_start(m_box, m_label, false, true, 0);

	gtk_widget_show_all(GTK_WIDGET(m_button));

	reload_icon_size();
}

//-----------------------------------------------------------------------------

SectionButton::~SectionButton()
{
	g_free(m_icon_name);
	gtk_widget_destroy(GTK_WIDGET(m_button));
}

//-----------------------------------------------------------------------------

void SectionButton::reload_icon_size()
{
	int size = wm_settings->category_icon_size.get_size();
	gtk_image_set_pixel_size(GTK_IMAGE(m_icon), size);
	gtk_widget_set_visible(m_icon, size > 1);

	if (wm_settings->category_show_name)
	{
		gtk_widget_set_has_tooltip(GTK_WIDGET(m_button), false);
		gtk_box_set_child_packing(m_box, m_icon, false, false, 0, GTK_PACK_START);
		gtk_widget_show(m_label);
	}
	else
	{
		gtk_widget_set_has_tooltip(GTK_WIDGET(m_button), true);
		gtk_widget_hide(m_label);
		gtk_box_set_child_packing(m_box, m_icon, true, true, 0, GTK_PACK_START);
	}
}

//-----------------------------------------------------------------------------
