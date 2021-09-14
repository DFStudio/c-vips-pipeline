cmake_minimum_required(VERSION 3.1)
project(vips-tool
        VERSION 1.0
        DESCRIPTION "A utility for processing images with vips"
        LANGUAGES C CXX)

IF(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local")
ENDIF()

set(CMAKE_CXX_STANDARD 17)

add_executable(vips-tool main.cpp unsharp.c unsharp.h operations.cpp operations.h Arguments.cpp Arguments.h MachineState.cpp MachineState.h)

target_include_directories(vips-tool PRIVATE include)
target_include_directories(vips-tool SYSTEM PUBLIC /usr/local/include /opt/homebrew/include)
target_link_directories(vips-tool PUBLIC /usr/local/lib /opt/homebrew/lib)

target_link_libraries(vips-tool PUBLIC -lvips -lvips-cpp)

find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
target_include_directories(vips-tool PRIVATE ${GOBJECT_INCLUDE_DIRS})
target_link_libraries(vips-tool PRIVATE fmt::fmt ${GOBJECT_LDFLAGS})