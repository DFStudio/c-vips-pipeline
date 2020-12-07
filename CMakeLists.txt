cmake_minimum_required(VERSION 3.1)
project(vips-scale
        VERSION 1.0
        DESCRIPTION "A utility for scaling to JPEG with VIPS"
        LANGUAGES C)

# https://cliutils.gitlab.io/modern-cmake/chapters/basics.html

add_executable(vips-scale main.c)

target_include_directories(vips-scale SYSTEM PUBLIC /usr/local/include)
target_link_directories(vips-scale PUBLIC /usr/local/lib)

target_link_libraries(vips-scale PUBLIC -lvips)

find_package(PkgConfig REQUIRED)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
target_include_directories(vips-scale PRIVATE ${GOBJECT_INCLUDE_DIRS})
target_link_libraries(vips-scale PRIVATE ${GOBJECT_LDFLAGS})