set(WHISKERMENU_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WHISKERMENU_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WHISKERMENU_FLAGS}")
if(ENABLE_VISIBILITY)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

if(ENABLE_AS_NEEDED)
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
endif()
if(ENABLE_LINKER_OPTIMIZED_HASH_TABLES)
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-O1")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.22)
pkg_check_modules(exo REQUIRED exo-2>=0.11)
pkg_check_modules(garcon REQUIRED garcon-1)
pkg_check_modules(libxfce4panel REQUIRED libxfce4panel-2.0>=4.11)
pkg_check_modules(libxfce4ui REQUIRED libxfce4ui-2>=4.11)
pkg_check_modules(libxfce4util REQUIRED libxfce4util-1.0>=4.11)

include_directories(
	${GTK3_INCLUDE_DIRS}
	${exo_INCLUDE_DIRS}
	${garcon_INCLUDE_DIRS}
	${libxfce4panel_INCLUDE_DIRS}
	${libxfce4ui_INCLUDE_DIRS}
	${libxfce4util_INCLUDE_DIRS})

link_directories(
	${GTK3_LIBRARY_DIRS}
	${exo_LIBRARY_DIRS}
	${garcon_LIBRARY_DIRS}
	${libxfce4panel_LIBRARY_DIRS}
	${libxfce4ui_LIBRARY_DIRS}
	${libxfce4util_LIBRARY_DIRS})

add_definitions(
	-DGETTEXT_PACKAGE="xfce4-whiskermenu-plugin"
	-DPACKAGE_LOCALE_DIR="${CMAKE_INSTALL_FULL_LOCALEDIR}"
	-DPACKAGE_NAME="xfce4-whiskermenu-plugin"
	-DPACKAGE_VERSION="${whiskermenu_version}"
	-DBINDIR="${CMAKE_INSTALL_FULL_BINDIR}"
	-DDATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
	-DSETTINGS_MENUFILE="${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/menus/xfce-settings-manager.menu"
	-DG_LOG_DOMAIN="whiskermenu"
	-DGSEAL_ENABLE
	-DGTK_MULTIHEAD_SAFE
	${GTK3_CFLAGS_OTHER}
	${exo_CFLAGS_OTHER}
	${garcon_CFLAGS_OTHER}
	${libxfce4panel_CFLAGS_OTHER}
	${libxfce4ui_CFLAGS_OTHER}
	${libxfce4util_CFLAGS_OTHER})

if(ENABLE_DEVELOPER_MODE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,defs")

	add_definitions(
		-DG_DISABLE_DEPRECATED
		-DGDK_DISABLE_DEPRECATED
		-DGDK_PIXBUF_DISABLE_DEPRECATED
		-DGTK_DISABLE_DEPRECATED
		-DGDK_DISABLE_SINGLE_INCLUDES
		-DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES
		-DGTK_DISABLE_SINGLE_INCLUDES)
endif()

add_library(whiskermenu MODULE
	applications-page.cpp
	category.cpp
	command.cpp
	command-edit.cpp
	configuration-dialog.cpp
	element.h
	favorites-page.cpp
	icon-size.cpp
	launcher.cpp
	launcher-tree-view.cpp
	page.cpp
	plugin.cpp
	profile-picture.cpp
	query.cpp
	recent-page.cpp
	register-plugin.c
	resizer-widget.cpp
	run-action.cpp
	search-action.cpp
	search-page.cpp
	section-button.cpp
	settings.cpp
	slot.h
	window.cpp)

target_link_libraries(whiskermenu
	${GTK3_LIBRARIES}
	${exo_LIBRARIES}
	${garcon_LIBRARIES}
	${libxfce4panel_LIBRARIES}
	${libxfce4ui_LIBRARIES}
	${libxfce4util_LIBRARIES})

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "release|minsizerel")
	add_definitions(-DNDEBUG -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT)
	add_custom_command(TARGET whiskermenu
		POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:whiskermenu>)
endif()

install(TARGETS whiskermenu LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/xfce4/panel/plugins)
install(FILES whiskermenu.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/xfce4/panel/plugins)

# popup script
configure_file(${PROJECT_SOURCE_DIR}/panel-plugin/xfce4-popup-whiskermenu.in
	${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu ESCAPE_QUOTES @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu
	DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES xfce4-popup-whiskermenu.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc)
