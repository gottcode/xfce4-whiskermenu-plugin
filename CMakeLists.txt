cmake_minimum_required(VERSION 2.8)
if(${CMAKE_VERSION} VERSION_LESS "2.8.8")
    cmake_policy(SET CMP0002 OLD)
endif()

project(whiskermenu)

# options
option(ENABLE_WERROR "Enable -Werror flag for development" OFF)
option(ENABLE_AS_NEEDED "Enable -Wl,--as-needed for the linker" ON)

include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(po)
add_subdirectory(icons)

configure_file(${PROJECT_SOURCE_DIR}/xfce4-popup-whiskermenu.in
    ${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu ESCAPE_QUOTES @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu DESTINATION ${CMAKE_INSTALL_BINDIR})

# uninstall target
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake)
