cmake_minimum_required(VERSION 3.9)

project(whiskermenu)

# version number
set(whiskermenu_version_major "2")
set(whiskermenu_version_minor "7")
set(whiskermenu_version_micro "0")
set(whiskermenu_version_tag "git")
set(whiskermenu_version "${whiskermenu_version_major}.${whiskermenu_version_minor}.${whiskermenu_version_micro}")
if(${whiskermenu_version_tag} MATCHES "git")
	if(NOT DEFINED whiskermenu_version_build)
		execute_process(COMMAND git describe
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE whiskermenu_version_build)
		string(REGEX REPLACE "^.*-g" "" whiskermenu_version_build "${whiskermenu_version_build}")
		string(REGEX REPLACE "\n" "" whiskermenu_version_build "${whiskermenu_version_build}")
	endif()
	set(whiskermenu_version "${whiskermenu_version}.${whiskermenu_version_tag}-${whiskermenu_version_build}")
endif()

# options
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "release|minsizerel")
	set(whiskermenu_strip ON)
endif()
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "release|minsizerel|relwithdebinfo")
	set(whiskermenu_lto ON)
endif()

option(ENABLE_ACCOUNTSSERVICE "Enable building with AccountsService" ON)
option(ENABLE_VISIBILITY "Enable ELF visibility attributes" ON)
option(ENABLE_AS_NEEDED "Enable -Wl,--as-needed for the linker" ON)
option(ENABLE_LINKER_OPTIMIZED_HASH_TABLES "Enable -Wl,-O1 for the linker" ON)
option(ENABLE_LINK_TIME_OPTIMIZATION "Enable link time optimization" ${whiskermenu_lto})
option(ENABLE_STRIP "Enable automatic stripping of release builds" ${whiskermenu_strip})
option(ENABLE_DEVELOPER_MODE "Enable strict checks to help with development" OFF)

include(GNUInstallDirs)

add_subdirectory(icons)
add_subdirectory(panel-plugin)
add_subdirectory(po)

# uninstall target
if(NOT TARGET uninstall)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake
		IMMEDIATE @ONLY)
	add_custom_target(uninstall
		${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake)
endif()
