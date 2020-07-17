find_package(Gettext REQUIRED)

# Generate translations
file(GLOB _po_files *.po)
foreach(_po_file ${_po_files})
	get_filename_component(_lang ${_po_file} NAME_WE)

	set(_gmo_file ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
	list(APPEND _gmo_files ${_gmo_file})

	add_custom_command(
		OUTPUT ${_gmo_file}
		COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmo_file} ${_po_file}
		DEPENDS ${_po_file}
	)
	install(
		FILES ${_gmo_file}
		DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/${_lang}/LC_MESSAGES/
		RENAME xfce4-whiskermenu-plugin.mo
	)
endforeach()
add_custom_target(translations ALL DEPENDS ${_gmo_files})
