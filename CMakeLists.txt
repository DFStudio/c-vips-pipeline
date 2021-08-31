cmake_minimum_required(VERSION 3.1)
project(vips-scale
        VERSION 1.0
        DESCRIPTION "A utility for processing images with vips"
        LANGUAGES C CXX)

IF(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local")
ENDIF()

set(CMAKE_CXX_STANDARD 17)

add_executable(vips-scale main.cpp unsharp.c unsharp.h operations.cpp operations.h Arguments.cpp Arguments.h)

target_include_directories(vips-scale SYSTEM PUBLIC /usr/local/include /opt/homebrew/include)
target_link_directories(vips-scale PUBLIC /usr/local/lib /opt/homebrew/lib)

target_link_libraries(vips-scale PUBLIC -lvips -lvips-cpp)

find_package(PkgConfig REQUIRED)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
target_include_directories(vips-scale PRIVATE ${GOBJECT_INCLUDE_DIRS})
target_link_libraries(vips-scale PRIVATE ${GOBJECT_LDFLAGS})