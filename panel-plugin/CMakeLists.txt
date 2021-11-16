find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0>=3.22)
pkg_check_modules(exo REQUIRED IMPORTED_TARGET exo-2>=0.11)
pkg_check_modules(garcon REQUIRED IMPORTED_TARGET garcon-1)
pkg_check_modules(libxfce4panel REQUIRED IMPORTED_TARGET libxfce4panel-2.0>=4.11)
pkg_check_modules(libxfce4ui REQUIRED IMPORTED_TARGET libxfce4ui-2>=4.11)
pkg_check_modules(libxfce4util REQUIRED IMPORTED_TARGET libxfce4util-1.0>=4.11)
if(ENABLE_ACCOUNTSSERVICE)
	pkg_check_modules(accountsservice IMPORTED_TARGET accountsservice>=0.6.45)
endif()

add_library(whiskermenu MODULE
	applications-page.cpp
	category.cpp
	category-button.cpp
	command.cpp
	command-edit.cpp
	element.cpp
	favorites-page.cpp
	icon-renderer.cpp
	icon-size.cpp
	image-menu-item.h
	launcher.cpp
	launcher-icon-view.cpp
	launcher-tree-view.cpp
	page.cpp
	plugin.cpp
	profile.cpp
	query.cpp
	recent-page.cpp
	register-plugin.c
	resizer.cpp
	run-action.cpp
	search-action.cpp
	search-page.cpp
	settings.cpp
	settings-dialog.cpp
	slot.h
	window.cpp)

target_link_libraries(whiskermenu PRIVATE
	PkgConfig::GTK3
	PkgConfig::exo
	PkgConfig::garcon
	PkgConfig::libxfce4panel
	PkgConfig::libxfce4ui
	PkgConfig::libxfce4util)

target_compile_options(whiskermenu PRIVATE -Wall -Wextra)

target_compile_definitions(whiskermenu PRIVATE
	GETTEXT_PACKAGE="xfce4-whiskermenu-plugin"
	PACKAGE_LOCALE_DIR="${CMAKE_INSTALL_FULL_LOCALEDIR}"
	PACKAGE_NAME="xfce4-whiskermenu-plugin"
	PACKAGE_VERSION="${whiskermenu_version}"
	BINDIR="${CMAKE_INSTALL_FULL_BINDIR}"
	DATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
	SETTINGS_MENUFILE="${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/menus/xfce-settings-manager.menu"
	G_LOG_DOMAIN="whiskermenu"
	GSEAL_ENABLE
	GTK_MULTIHEAD_SAFE
	$<$<CONFIG:Release>:G_DISABLE_ASSERT G_DISABLE_CAST_CHECKS>
	$<$<CONFIG:MinSizeRel>:G_DISABLE_ASSERT G_DISABLE_CAST_CHECKS>
	$<$<CONFIG:RelWithDebInfo>:G_DISABLE_ASSERT G_DISABLE_CAST_CHECKS>)

target_compile_features(whiskermenu PRIVATE cxx_std_11)

if (accountsservice_FOUND)
	target_link_libraries(whiskermenu PRIVATE
		PkgConfig::accountsservice)
	target_compile_definitions(whiskermenu PRIVATE
		HAS_ACCOUNTSERVICE)
endif()

if(ENABLE_VISIBILITY)
	set_target_properties(whiskermenu PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
	if(ENABLE_AS_NEEDED)
		target_link_options(whiskermenu PRIVATE "LINKER:--as-needed")
	endif()

	if(ENABLE_LINKER_OPTIMIZED_HASH_TABLES)
		target_link_options(whiskermenu PRIVATE "LINKER:-O1")
	endif()

	if(ENABLE_DEVELOPER_MODE)
		target_link_options(whiskermenu PRIVATE "LINKER:-z,defs")
	endif()
else()
	if(ENABLE_AS_NEEDED)
		set_property(TARGET whiskermenu APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--as-needed")
	endif()

	if(ENABLE_LINKER_OPTIMIZED_HASH_TABLES)
		set_property(TARGET whiskermenu APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-O1")
	endif()

	if(ENABLE_DEVELOPER_MODE)
		set_property(TARGET whiskermenu APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-z,defs")
	endif()
endif()

if(ENABLE_DEVELOPER_MODE)
	target_compile_options(whiskermenu PRIVATE -Werror)
	target_compile_definitions(whiskermenu PRIVATE
		XFCE_DISABLE_DEPRECATED
		G_DISABLE_DEPRECATED
		GDK_DISABLE_DEPRECATED
		GDK_PIXBUF_DISABLE_DEPRECATED
		GDK_DISABLE_SINGLE_INCLUDES
		GDK_PIXBUF_DISABLE_SINGLE_INCLUDES
		GTK_DISABLE_SINGLE_INCLUDES)
endif()

if(ENABLE_LINK_TIME_OPTIMIZATION)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if(result)
		set_target_properties(whiskermenu PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

if(ENABLE_STRIP)
	add_custom_command(TARGET whiskermenu POST_BUILD
		COMMAND ${CMAKE_STRIP} $<TARGET_FILE:whiskermenu>)
endif()

install(TARGETS whiskermenu LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/xfce4/panel/plugins)

# popup script
configure_file(${PROJECT_SOURCE_DIR}/panel-plugin/xfce4-popup-whiskermenu.in
	${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu ESCAPE_QUOTES @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu
	DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES xfce4-popup-whiskermenu.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc)
