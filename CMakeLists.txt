cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

project(whiskermenu)

if(NOT DEFINED LIB_INSTALL_DIR)
    if (DEFINED CMAKE_INSTALL_LIBDIR)
        set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
    else()
        set(LIB_INSTALL_DIR lib)
    endif()
endif()

if(NOT DEFINED DATA_INSTALL_DIR)
    if (DEFINED CMAKE_DATA_PATH)
        set(DATA_INSTALL_DIR ${CMAKE_DATA_PATH})
    else()
        set(DATA_INSTALL_DIR share)
    endif()
endif()

if(NOT DEFINED ICON_INSTALL_DIR)
    set(ICON_INSTALL_DIR ${DATA_INSTALL_DIR}/icons)
endif()

if(NOT DEFINED LOCALE_INSTALL_DIR)
    if (DEFINED CMAKE_LOCALE_PATH)
        set(LOCALE_INSTALL_DIR ${CMAKE_LOCALE_PATH})
    else()
        set(LOCALE_INSTALL_DIR ${DATA_INSTALL_DIR}/locale)
    endif()
endif()

add_subdirectory(src)
add_subdirectory(po)
add_subdirectory(icons)

# uninstall target
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake)
