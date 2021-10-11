/*
 * Copyright (C) 2014-2021 Graeme Gott <graeme@gottcode.org>
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

#include "profile-picture.h"

#include "command.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

#include <libxfce4panel/libxfce4panel.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

ProfilePicture::ProfilePicture(Window* window) :
	m_window(window)
{
	m_image = gtk_image_new();

	gtk_widget_set_halign(m_image, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(m_image, GTK_ALIGN_CENTER);

	m_container = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(m_container), false);
	gtk_widget_add_events(m_container, GDK_BUTTON_PRESS_MASK);
	g_signal_connect_slot<GtkWidget*, GdkEvent*>(m_container, "button-press-event", &ProfilePicture::on_button_press_event, this);
	gtk_container_add(GTK_CONTAINER(m_container), m_image);

	Command* command = wm_settings->command[Settings::CommandProfile];
	gtk_widget_set_tooltip_text(m_container, command->get_tooltip());

#ifdef HAS_ACCOUNTSERVICE
	m_act_user_manager = act_user_manager_get_default();
	gboolean loaded = FALSE;
	g_object_get(m_act_user_manager, "is-loaded", &loaded, nullptr);
	if (loaded)
	{
		on_user_info_loaded(m_act_user_manager, nullptr);
	}
	else
	{
		g_signal_connect_slot(m_act_user_manager, "notify::is-loaded", &ProfilePicture::on_user_info_loaded, this);
	}
#else
	m_file_path = g_build_filename(g_get_home_dir(), ".face", nullptr);

	GFile* file = g_file_new_for_path(m_file_path);
	m_file_monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, nullptr, nullptr);
	g_signal_connect_slot(m_file_monitor, "changed", &ProfilePicture::on_file_changed, this);
	on_file_changed(m_file_monitor, nullptr, nullptr, G_FILE_MONITOR_EVENT_CHANGED);

	g_object_unref(file);
#endif
}

//-----------------------------------------------------------------------------

ProfilePicture::~ProfilePicture()
{
#ifdef HAS_ACCOUNTSERVICE
	g_object_unref(m_act_user_manager);
	g_object_unref(m_act_user);
#else
	g_file_monitor_cancel(m_file_monitor);
	g_object_unref(m_file_monitor);
#endif
	if (m_file_path)
	{
		g_free(m_file_path);
	}
}

//-----------------------------------------------------------------------------

void ProfilePicture::reset_tooltip()
{
	Command* command = wm_settings->command[Settings::CommandProfile];
	gtk_widget_set_has_tooltip(m_container, command->get_shown());
}

//-----------------------------------------------------------------------------

void ProfilePicture::update_profile_picture()
{
	const gint size = 32;

	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_size(m_file_path, size, size, nullptr);
	if (!pixbuf)
	{
		gtk_image_set_from_icon_name(GTK_IMAGE(m_image), "avatar-default", GTK_ICON_SIZE_DND);
		return;
	}

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
	cairo_t* cr = cairo_create(surface);

	cairo_arc(cr, size/2, size/2, size/2, 0, 2 * G_PI);
	cairo_clip(cr);
	cairo_new_path(cr);

	gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
	cairo_paint(cr);

	GdkPixbuf* dest = gdk_pixbuf_get_from_surface(surface, 0, 0, size, size);
	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	g_object_unref(pixbuf);

	gtk_image_set_from_pixbuf(GTK_IMAGE(m_image), dest);
	g_object_unref(dest);
}

#ifdef HAS_ACCOUNTSERVICE
//-----------------------------------------------------------------------------

void ProfilePicture::on_user_changed(ActUserManager*, ActUser* user)
{
	if (act_user_get_uid(user) != getuid())
	{
		return;
	}

	if (m_file_path)
	{
		g_free(m_file_path);
	}

	m_file_path = g_strdup(act_user_get_icon_file(user));

	update_profile_picture();
}

//-----------------------------------------------------------------------------

void ProfilePicture::on_user_loaded(ActUser* user, GParamSpec*)
{
	on_user_changed(nullptr, user);
}

//-----------------------------------------------------------------------------

void ProfilePicture::on_user_info_loaded(ActUserManager*, GParamSpec*)
{
	if (act_user_manager_no_service(m_act_user_manager))
	{
		gtk_image_set_from_icon_name(GTK_IMAGE(m_image), "avatar-default", GTK_ICON_SIZE_DND);
		return;
	}

	g_signal_connect_slot(m_act_user_manager, "user-changed", &ProfilePicture::on_user_changed, this);

	m_act_user = act_user_manager_get_user_by_id(m_act_user_manager, getuid());
	if (act_user_is_loaded(m_act_user))
	{
		on_user_changed(nullptr, m_act_user);
	}
	else
	{
		g_signal_connect_slot(m_act_user, "notify::is-loaded", &ProfilePicture::on_user_loaded, this);
	}
}
#else
void ProfilePicture::on_file_changed(GFileMonitor*, GFile*, GFile*, GFileMonitorEvent)
{
	update_profile_picture();
}
#endif

//-----------------------------------------------------------------------------

void ProfilePicture::on_button_press_event()
{
	Command* command = wm_settings->command[Settings::CommandProfile];
	if (!command->get_shown())
	{
		return;
	}

	m_window->hide();
	command->activate();
}

//-----------------------------------------------------------------------------
