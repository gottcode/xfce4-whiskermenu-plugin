/*
 * Copyright (C) 2014 Graeme Gott <graeme@gottcode.org>
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

#include "slot.h"

#include <libxfce4panel/libxfce4panel.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

ProfilePicture::ProfilePicture()
{
	m_image = xfce_panel_image_new();

	gchar* path = g_build_filename(g_get_home_dir(), ".face", NULL);
	GFile* file = g_file_new_for_path(path);
	g_free(path);

	m_file_monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, NULL);
	g_signal_connect_slot(m_file_monitor, "changed", &ProfilePicture::on_file_changed, this);
	on_file_changed(m_file_monitor, file, NULL, G_FILE_MONITOR_EVENT_CHANGED);

	g_object_unref(file);

	m_alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(m_alignment), 0, 0, 10, 10);
	gtk_container_add(GTK_CONTAINER(m_alignment), m_image);
}

//-----------------------------------------------------------------------------

ProfilePicture::~ProfilePicture()
{
	g_file_monitor_cancel(m_file_monitor);
	g_object_unref(m_file_monitor);
}

//-----------------------------------------------------------------------------

void ProfilePicture::on_file_changed(GFileMonitor*, GFile* file, GFile*, GFileMonitorEvent)
{
	gint width = 32, height = 32;
	gtk_icon_size_lookup(GTK_ICON_SIZE_DND, &width, &height);

	gchar* path = g_file_get_path(file);
	GdkPixbuf* face = gdk_pixbuf_new_from_file_at_size(path, width, height, NULL);
	g_free(path);

	XfcePanelImage* image = XFCE_PANEL_IMAGE(m_image);
	if (face)
	{
		xfce_panel_image_set_size(image, -1);
		xfce_panel_image_set_from_pixbuf(image, face);
		g_object_unref(face);
	}
	else
	{
		xfce_panel_image_set_size(image, height);
		xfce_panel_image_set_from_source(image, "avatar-default");
	}
}

//-----------------------------------------------------------------------------
