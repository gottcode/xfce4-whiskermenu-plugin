cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

project(whiskermenu)

include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(po)
add_subdirectory(icons)

configure_file(${PROJECT_SOURCE_DIR}/xfce4-popup-whiskermenu.in
    ${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu ESCAPE_QUOTES @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/xfce4-popup-whiskermenu DESTINATION ${CMAKE_INSTALL_BINDIR})
