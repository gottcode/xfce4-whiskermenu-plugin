/*
 * Copyright (C) 2021 Graeme Gott <graeme@gottcode.org>
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

#include <iostream>

#include <gio/gio.h>
#include <glib/gi18n.h>

int main(int argc, char** argv)
{
	// Set up gettext
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	// Parse commandline options
	gboolean at_pointer = FALSE;
	gboolean print_version = FALSE;
	const GOptionEntry entries[] =
	{
		{ "pointer", 'p', 0, G_OPTION_ARG_NONE, &at_pointer, _("Popup menu at current mouse position"), nullptr },
		{ "version", 'V', 0, G_OPTION_ARG_NONE, &print_version, _("Print version information and exit"), nullptr },
		{ nullptr, '\0', 0, G_OPTION_ARG_NONE, nullptr, nullptr, nullptr }
	};

	GError* error = nullptr;
	GOptionContext* context = g_option_context_new(nullptr);
	g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		std::cerr << PACKAGE_NAME << ": " << error->message << std::endl;
		g_error_free(error);
		return EXIT_FAILURE;
	}
	g_option_context_free(context);

	if (print_version)
	{
		std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << "\n"
				<< _("Copyright \302\251 2013-2021 Graeme Gott") << std::endl;
		return EXIT_SUCCESS;
	}

	// Connect to Xfce panel through D-Bus
	error = nullptr;
	GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			nullptr,
			"org.xfce.Panel",
			"/org/xfce/Panel",
			"org.xfce.Panel",
			nullptr,
			&error);
	if (!proxy)
	{
		std::cerr << PACKAGE_NAME << ": " << error->message << std::endl;
		g_error_free(error);
		return EXIT_FAILURE;
	}

	// Tell panel to call remote-event on Whisker Menu
	int result = EXIT_SUCCESS;
	GVariant* variant = g_variant_new_variant(g_variant_new_boolean(at_pointer));
	GVariant* ret = g_dbus_proxy_call_sync(proxy,
			"PluginEvent",
			g_variant_new("(ss@v)", "whiskermenu", "popup", variant),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			nullptr,
			&error);
	if (!ret)
	{
		std::cerr << PACKAGE_NAME << ": " << error->message << std::endl;
		g_error_free(error);
		result = EXIT_FAILURE;
	}
	else
	{
		g_variant_unref(ret);
	}

	// Disconnect from D-Bus
	g_object_unref(G_OBJECT(proxy));

	return result;
}
