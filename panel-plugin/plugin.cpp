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

#include "plugin.h"

#include "applications-page.h"
#include "command.h"
#include "configuration-dialog.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

extern "C"
{
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

extern "C" void whiskermenu_construct(XfcePanelPlugin* plugin)
{
	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
	new Plugin(plugin);
}

static void whiskermenu_free(XfcePanelPlugin*, Plugin* whiskermenu)
{
	delete whiskermenu;
	whiskermenu = NULL;
}

// Wait for grab; allows modifier as shortcut
// Adapted from http://git.xfce.org/xfce/xfce4-panel/tree/common/panel-utils.c#n122
static bool panel_utils_grab_available()
{
	bool grab_succeed = false;

	// Don't try to get the grab for longer then 1/4 second
	GdkWindow* root = gdk_screen_get_root_window(xfce_gdk_screen_get_active(NULL));
	GdkGrabStatus grab_pointer = GDK_GRAB_FROZEN;
	GdkGrabStatus grab_keyboard = GDK_GRAB_FROZEN;
	for (guint i = 0; i < (G_USEC_PER_SEC / 4 / 100); ++i)
	{
		grab_keyboard = gdk_keyboard_grab(root, true, GDK_CURRENT_TIME);
		if (grab_keyboard == GDK_GRAB_SUCCESS)
		{
			const GdkEventMask pointer_mask = static_cast<GdkEventMask>(GDK_BUTTON_PRESS_MASK
					| GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK
					| GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK);
			grab_pointer = gdk_pointer_grab(root, true, pointer_mask, NULL, NULL, GDK_CURRENT_TIME);
			if (grab_pointer == GDK_GRAB_SUCCESS)
			{
				grab_succeed = true;
				break;
			}
		}
		g_usleep(100);
	}

	// Release the grab so the menu window can take it
	if (grab_pointer == GDK_GRAB_SUCCESS)
	{
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
	}
	if (grab_keyboard == GDK_GRAB_SUCCESS)
	{
		gdk_keyboard_ungrab(GDK_CURRENT_TIME);
	}

	if (!grab_succeed)
	{
		g_printerr("xfce4-whiskermenu-plugin: Unable to get keyboard and mouse grab. Menu popup failed.\n");
	}

	return grab_succeed;
}

//-----------------------------------------------------------------------------

Plugin::Plugin(XfcePanelPlugin* plugin) :
	m_plugin(plugin),
	m_window(NULL)
{
	// Load settings
	wm_settings = new Settings;
	wm_settings->button_title = get_button_title_default();
	wm_settings->load(g_strconcat(DATADIR, "/xfce4/whiskermenu/defaults.rc", NULL));
	wm_settings->load(xfce_panel_plugin_lookup_rc_file(m_plugin));

	// Prevent empty panel button
	if (!wm_settings->button_icon_visible)
	{
		if (!wm_settings->button_title_visible)
		{
			wm_settings->button_icon_visible = true;
		}
		else if (wm_settings->button_title.empty())
		{
			wm_settings->button_title = get_button_title_default();
		}
	}

	// Create menu window
	m_window = new Window;
	g_signal_connect_slot<GtkWidget*>(m_window->get_widget(), "unmap", &Plugin::menu_hidden, this);

	// Create toggle button
	m_button = xfce_panel_create_toggle_button();
	gtk_widget_set_name(m_button, "whiskermenu-button");
	gtk_button_set_relief(GTK_BUTTON(m_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(m_button), false);
	g_signal_connect_slot(m_button, "button-press-event", &Plugin::button_clicked, this);
	gtk_widget_show(m_button);

	m_button_box = GTK_BOX(gtk_hbox_new(false, 2));
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(m_button_box));
	gtk_container_set_border_width(GTK_CONTAINER(m_button_box), 0);
	gtk_widget_show(GTK_WIDGET(m_button_box));

	m_button_icon = GTK_IMAGE(gtk_image_new());
	gtk_box_pack_start(m_button_box, GTK_WIDGET(m_button_icon), true, false, 0);
	if (wm_settings->button_icon_visible)
	{
		gtk_widget_show(GTK_WIDGET(m_button_icon));
	}

	m_button_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_markup(m_button_label, wm_settings->button_title.c_str());
	gtk_box_pack_start(m_button_box, GTK_WIDGET(m_button_label), true, true, 0);
	if (wm_settings->button_title_visible)
	{
		gtk_widget_show(GTK_WIDGET(m_button_label));
	}

	// Add plugin to panel
	gtk_container_add(GTK_CONTAINER(plugin), m_button);
	xfce_panel_plugin_add_action_widget(plugin, m_button);

	// Connect plugin signals to functions
	g_signal_connect(plugin, "free-data", G_CALLBACK(whiskermenu_free), this);
	g_signal_connect_slot<XfcePanelPlugin*>(plugin, "configure-plugin", &Plugin::configure, this);
#if (LIBXFCE4PANEL_CHECK_VERSION(4,9,0))
	g_signal_connect_slot(plugin, "mode-changed", &Plugin::mode_changed, this);
#else
	g_signal_connect_slot(plugin, "orientation-changed", &Plugin::orientation_changed, this);
#endif
	g_signal_connect_slot(plugin, "remote-event", &Plugin::remote_event, this);
	g_signal_connect_slot<XfcePanelPlugin*>(plugin, "save", &Plugin::save, this);
	g_signal_connect_slot<XfcePanelPlugin*>(plugin, "about", &Plugin::show_about, this);
	g_signal_connect_slot(plugin, "size-changed", &Plugin::size_changed, this);

	xfce_panel_plugin_menu_show_about(plugin);
	xfce_panel_plugin_menu_show_configure(plugin);
	xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(wm_settings->command[Settings::CommandMenuEditor]->get_menuitem()));

#if (LIBXFCE4PANEL_CHECK_VERSION(4,9,0))
	mode_changed(m_plugin, xfce_panel_plugin_get_mode(m_plugin));
#else
	orientation_changed(m_plugin, xfce_panel_plugin_get_orientation(m_plugin));
#endif
}

//-----------------------------------------------------------------------------

Plugin::~Plugin()
{
	save();

	delete m_window;
	m_window = NULL;

	delete wm_settings;
	wm_settings = NULL;

	gtk_widget_destroy(m_button);
}

//-----------------------------------------------------------------------------

Plugin::ButtonStyle Plugin::get_button_style() const
{
	return ButtonStyle(wm_settings->button_icon_visible | (wm_settings->button_title_visible << 1));
}

//-----------------------------------------------------------------------------

std::string Plugin::get_button_title() const
{
	return wm_settings->button_title;
}

//-----------------------------------------------------------------------------

std::string Plugin::get_button_title_default()
{
	return _("Applications");
}

//-----------------------------------------------------------------------------

std::string Plugin::get_button_icon_name() const
{
	return wm_settings->button_icon_name;
}

//-----------------------------------------------------------------------------

void Plugin::reload()
{
	m_window->hide();
	m_window->get_applications()->invalidate_applications();
}

//-----------------------------------------------------------------------------

void Plugin::set_button_style(ButtonStyle style)
{
	wm_settings->button_icon_visible = style & ShowIcon;
	if (wm_settings->button_icon_visible)
	{
		gtk_widget_show(GTK_WIDGET(m_button_icon));
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET(m_button_icon));
	}

	wm_settings->button_title_visible = style & ShowText;
	if (wm_settings->button_title_visible)
	{
		gtk_widget_show(GTK_WIDGET(m_button_label));
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET(m_button_label));
	}

	wm_settings->set_modified();

	size_changed(m_plugin, xfce_panel_plugin_get_size(m_plugin));
}

//-----------------------------------------------------------------------------

void Plugin::set_button_title(const std::string& title)
{
	wm_settings->button_title = title;
	wm_settings->set_modified();
	gtk_label_set_markup(m_button_label, wm_settings->button_title.c_str());
	size_changed(m_plugin, xfce_panel_plugin_get_size(m_plugin));
}

//-----------------------------------------------------------------------------

void Plugin::set_button_icon_name(const std::string& icon)
{
	wm_settings->button_icon_name = icon;
	wm_settings->set_modified();
	size_changed(m_plugin, xfce_panel_plugin_get_size(m_plugin));
}

//-----------------------------------------------------------------------------

void Plugin::set_configure_enabled(bool enabled)
{
	if (enabled)
	{
		xfce_panel_plugin_unblock_menu(m_plugin);
	}
	else
	{
		xfce_panel_plugin_block_menu(m_plugin);
	}
}

//-----------------------------------------------------------------------------

gboolean Plugin::button_clicked(GtkWidget*, GdkEvent* event)
{
	GdkEventButton* event_button = reinterpret_cast<GdkEventButton*>(event);
	if (event_button->button != 1 || event_button->state & GDK_CONTROL_MASK)
	{
		return false;
	}

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_button)) == true)
	{
		m_window->hide();
	}
	else
	{
		popup_menu(false, false);
	}

	return true;
}

//-----------------------------------------------------------------------------

void Plugin::menu_hidden()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button), false);
	xfce_panel_plugin_block_autohide(m_plugin, false);
	save();
}

//-----------------------------------------------------------------------------

void Plugin::configure()
{
	ConfigurationDialog* dialog = new ConfigurationDialog(this);
	g_signal_connect_slot<GtkObject*>(dialog->get_widget(), "destroy", &Plugin::save, this);
}

//-----------------------------------------------------------------------------

#if (LIBXFCE4PANEL_CHECK_VERSION(4,9,0))
void Plugin::mode_changed(XfcePanelPlugin*, XfcePanelPluginMode mode)
{
	gtk_label_set_angle(m_button_label, (mode == XFCE_PANEL_PLUGIN_MODE_VERTICAL) ? 270: 0);
	size_changed(m_plugin, xfce_panel_plugin_get_size(m_plugin));
}
#else
void Plugin::orientation_changed(XfcePanelPlugin*, GtkOrientation orientation)
{
	gtk_label_set_angle(m_button_label, (orientation == GTK_ORIENTATION_VERTICAL) ? 270: 0);
	size_changed(m_plugin, xfce_panel_plugin_get_size(m_plugin));
}
#endif

//-----------------------------------------------------------------------------

gboolean Plugin::remote_event(XfcePanelPlugin*, gchar* name, GValue* value)
{
	if (strcmp(name, "popup") || !panel_utils_grab_available())
	{
		return false;
	}

	if (gtk_widget_get_visible(m_window->get_widget()))
	{
		m_window->hide();
	}
	else
	{
		popup_menu(value && G_VALUE_HOLDS_BOOLEAN(value) && g_value_get_boolean(value), true);
	}

	return true;
}

//-----------------------------------------------------------------------------

void Plugin::save()
{
	m_window->save();

	if (wm_settings->get_modified())
	{
		wm_settings->save(xfce_panel_plugin_save_location(m_plugin, true));
	}
}

//-----------------------------------------------------------------------------

void Plugin::show_about()
{
	const gchar* authors[] = {
		"Graeme Gott <graeme@gottcode.org>",
		NULL };

	gtk_show_about_dialog
		(NULL,
		"authors", authors,
		"comments", _("Alternate application launcher for Xfce"),
		"copyright", _("Copyright \302\251 2013-2014 Graeme Gott"),
		"license", XFCE_LICENSE_GPL,
		"logo-icon-name", "xfce4-whiskermenu",
		"program-name", PACKAGE_NAME,
		"translator-credits", _("translator-credits"),
		"version", PACKAGE_VERSION,
		"website", "http://gottcode.org/xfce4-whiskermenu-plugin/",
		NULL);
}

//-----------------------------------------------------------------------------

gboolean Plugin::size_changed(XfcePanelPlugin*, gint size)
{
	GtkOrientation panel_orientation = xfce_panel_plugin_get_orientation(m_plugin);
	GtkOrientation orientation = panel_orientation;
#if (LIBXFCE4PANEL_CHECK_VERSION(4,9,0))
	gint row_size = size / xfce_panel_plugin_get_nrows(m_plugin);
	XfcePanelPluginMode mode = xfce_panel_plugin_get_mode(m_plugin);
#else
	gint row_size = size;
#endif

	// Make icon expand to fill button if title is not visible
	gtk_box_set_child_packing(GTK_BOX(m_button_box), GTK_WIDGET(m_button_icon),
			!wm_settings->button_title_visible,
			!wm_settings->button_title_visible,
			0, GTK_PACK_START);

	// Load icon
	GtkStyle* style = gtk_widget_get_style(m_button);
	gint border = (2 * std::max(style->xthickness, style->ythickness)) + 2;

	GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(m_plugin));
	GtkIconTheme* icon_theme = G_LIKELY(screen != NULL) ? gtk_icon_theme_get_for_screen(screen) : NULL;

#if (LIBXFCE4PANEL_CHECK_VERSION(4,9,0))
	gint icon_width_max = (mode == XFCE_PANEL_PLUGIN_MODE_HORIZONTAL) ?
			6 * row_size - border :
			size - border;
	gint icon_height_max = row_size - border;
	GdkPixbuf* icon = xfce_panel_pixbuf_from_source_at_size(
			wm_settings->button_icon_name.c_str(),
			icon_theme,
			icon_width_max,
			icon_height_max);
#else
	GdkPixbuf* icon = xfce_panel_pixbuf_from_source(
			wm_settings->button_icon_name.c_str(),
			icon_theme,
			row_size - border);
#endif
	gint icon_width = 0;
	if (G_LIKELY(icon != NULL))
	{
		gtk_image_set_from_pixbuf(m_button_icon, icon);
		icon_width = gdk_pixbuf_get_width(icon);
		g_object_unref(G_OBJECT(icon));
	}

#if (LIBXFCE4PANEL_CHECK_VERSION(4,9,0))
	if (wm_settings->button_title_visible || !wm_settings->button_single_row)
	{
		xfce_panel_plugin_set_small(m_plugin, false);

		// Put title next to icon if panel is wide enough
		GtkRequisition label_size;
		gtk_widget_size_request(GTK_WIDGET(m_button_label), &label_size);
		if (mode == XFCE_PANEL_PLUGIN_MODE_DESKBAR &&
				wm_settings->button_title_visible &&
				wm_settings->button_icon_visible &&
				label_size.width <= (size - border - icon_width))
		{
			orientation = GTK_ORIENTATION_HORIZONTAL;
		}
	}
	else
	{
		xfce_panel_plugin_set_small(m_plugin, true);
	}
#endif

	// Fix alignment in deskbar mode
	if ((panel_orientation == GTK_ORIENTATION_VERTICAL) && (orientation == GTK_ORIENTATION_HORIZONTAL))
	{
		gtk_box_set_child_packing(m_button_box, GTK_WIDGET(m_button_label), false, false, 0, GTK_PACK_START);
	}
	else
	{
		gtk_box_set_child_packing(m_button_box, GTK_WIDGET(m_button_label), true, true, 0, GTK_PACK_START);
	}

	gtk_orientable_set_orientation(GTK_ORIENTABLE(m_button_box), orientation);

	return true;
}

//-----------------------------------------------------------------------------

void Plugin::popup_menu(bool at_cursor, bool activate_button)
{
	if (!at_cursor)
	{
		xfce_panel_plugin_block_autohide(m_plugin, true);
		if (activate_button)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button), true);
		}
		m_window->show(m_button, xfce_panel_plugin_get_orientation(m_plugin) == GTK_ORIENTATION_HORIZONTAL);
	}
	else
	{
		m_window->show(NULL, true);
	}
}

//-----------------------------------------------------------------------------
