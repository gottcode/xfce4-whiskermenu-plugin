find_package(Gettext REQUIRED)
file(GLOB _po_files *.po)
GETTEXT_CREATE_TRANSLATIONS(xfce4-whiskermenu-plugin.pot ALL ${_po_files})
