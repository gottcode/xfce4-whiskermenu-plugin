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

find_package(GTK2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(exo REQUIRED exo-1)
pkg_check_modules(garcon REQUIRED garcon-1)
pkg_check_modules(libxfce4panel REQUIRED libxfce4panel-1.0>=4.7)
pkg_check_modules(libxfce4ui REQUIRED libxfce4ui-1>=4.7)
pkg_check_modules(libxfce4util REQUIRED libxfce4util-1.0>=4.7)

include_directories(
	${exo_INCLUDE_DIRS}
	${garcon_INCLUDE_DIRS}
	${libxfce4panel_INCLUDE_DIRS}
	${libxfce4ui_INCLUDE_DIRS}
	${libxfce4util_INCLUDE_DIRS}
	${GTK2_INCLUDE_DIRS})

link_directories(
	${exo_LIBRARY_DIRS}
	${garcon_LIBRARY_DIRS}
	${libxfce4panel_LIBRARY_DIRS}
	${libxfce4ui_LIBRARY_DIRS}
	${libxfce4util_LIBRARY_DIRS})

add_definitions(
	-DGETTEXT_PACKAGE="xfce4-whiskermenu-plugin"
	-DPACKAGE_LOCALE_DIR="${CMAKE_INSTALL_FULL_LOCALEDIR}"
	-DBINDIR="${CMAKE_INSTALL_FULL_BINDIR}"
	-DDATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
	-DG_LOG_DOMAIN="whiskermenu"
	-DGSEAL_ENABLE
	-DGTK_MULTIHEAD_SAFE
	${exo_CFLAGS_OTHER}
	${garcon_CFLAGS_OTHER}
	${libxfce4panel_CFLAGS_OTHER}
	${libxfce4ui_CFLAGS_OTHER}
	${libxfce4util_CFLAGS_OTHER})

if(ENABLE_DEVELOPER_MODE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,defs")

	if(${libxfce4panel_VERSION} VERSION_GREATER "4.9")
		add_definitions(
			-DG_DISABLE_DEPRECATED
			-DGDK_DISABLE_DEPRECATED
			-DGDK_PIXBUF_DISABLE_DEPRECATED
			-DGTK_DISABLE_DEPRECATED)
	endif()
	add_definitions(
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
	launcher-view.cpp
	list-page.cpp
	page.cpp
	plugin.cpp
	query.cpp
	recent-page.cpp
	register-plugin.c
	resizer-widget.cpp
	search-page.cpp
	section-button.cpp
	settings.cpp
	window.cpp)

target_link_libraries(whiskermenu
	${exo_LIBRARIES}
	${garcon_LIBRARIES}
	${libxfce4panel_LIBRARIES}
	${libxfce4ui_LIBRARIES}
	${libxfce4util_LIBRARIES}
	${GTK2_LIBRARIES})

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
