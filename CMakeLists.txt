cmake_minimum_required(VERSION 3.17)
project(vips-scale
        VERSION 1.0
        DESCRIPTION "A utility for scaling to JPEG with VIPS"
        LANGUAGES C)

# https://cliutils.gitlab.io/modern-cmake/chapters/basics.html

#find_library(VIPS_LIB NAMES vips)
#find_library(GOBJECT_LIB NAMES gobject-2.0)
#find_library(GLIB_LIB NAMES glib-2.0)
#find_library(INTL_LIB NAMES intl)

add_executable(vips-scale main.c)

#target_include_directories(vips-scale SYSTEM PUBLIC ${VIPS_INCLUDE_DIRS})

target_link_libraries(vips-scale ${VIPS_LIB} ${GOBJECT_LIB} ${GLIB_LIB} ${INTL_LIB})