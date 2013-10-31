cmake_minimum_required(VERSION 2.8)
if(${CMAKE_VERSION} VERSION_LESS "2.8.8")
	cmake_policy(SET CMP0002 OLD)
endif()

project(whiskermenu)

# version number
set(whiskermenu_version_major "1")
set(whiskermenu_version_minor "2")
set(whiskermenu_version_micro "0")
set(whiskermenu_version_tag "git")
set(whiskermenu_version "${whiskermenu_version_major}.${whiskermenu_version_minor}.${whiskermenu_version_micro}")
if(${whiskermenu_version_tag} MATCHES "git")
	if(NOT DEFINED whiskermenu_version_build)
		execute_process(COMMAND git describe
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE whiskermenu_version_build)
		string(REGEX REPLACE "^.*-g" "" whiskermenu_version_build "${whiskermenu_version_build}")
	endif()
	set(whiskermenu_version "${whiskermenu_version}.${whiskermenu_version_tag}-${whiskermenu_version_build}")
endif()

# options
option(ENABLE_WERROR "Enable -Werror flag for development" OFF)
option(ENABLE_AS_NEEDED "Enable -Wl,--as-needed for the linker" ON)
option(ENABLE_LINKER_OPTIMIZED_HASH_TABLES "Enable -Wl,-O1 for the linker" ON)

include(GNUInstallDirs)

add_subdirectory(icons)
add_subdirectory(panel-plugin)
add_subdirectory(po)

# uninstall target
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake
	IMMEDIATE @ONLY)
add_custom_target(uninstall
	${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake)
