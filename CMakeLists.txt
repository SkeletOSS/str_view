# File sets are available in 3.23 onward. Valuable tool.
cmake_minimum_required(VERSION 3.23)

set(namespace "str_view")
project("str_view"
    VERSION 0.8.1
    LANGUAGES C
    DESCRIPTION "Robust read-only string handling, tokenization, and matching in C."
)


# For the sake of vcpkg we will make sure cloning and building from
# main will only build the minimal files in the all target. I can't
# protect vcpkg from needlessly downloading 
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/samples")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/samples" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/samples")
    include(tools/scanners.cmake)
endif()

if (NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "This project has a top-level one called [${CMAKE_PROJECT_NAME}]")
else()
    message(STATUS "This project is a top-level one")
endif()

option(BUILD_SHARED_LIBS "Building str_view as a shared library is available, but OFF by default." OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/debug/bin)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

###############################################################################
################### SV Library Target For Consumers  #########################
# Even though we have directories for tests, samples, and utilities, these will
# all be stripped out of releases. So we treat this project as if it's sole
# purpose is to distribute only the core str_view::str_view target and create the library
# target at the root with relative pathing such that the project will compile
# on a user's system regardless of being fetched via FetchContent or 
# find_package. This way they also get to include our headers with the str_view 
# prefix, such as `#include "str_view/flat_buffer.h"`. Other directories will be 
# conditionally detected.
###############################################################################
add_library(${PROJECT_NAME})
add_library(${namespace}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/${PROJECT_NAME}.c
    PUBLIC 
        FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
            FILES 
                str_view/${PROJECT_NAME}.h
                str_view/configuration.h
)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)
if (BUILD_SHARED_LIBS AND WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SV_BUILD_DLL=1)
endif()
# set properties for the target. VERSION set the library version to the project
# version * SOVERSION set the compatibility  version for the library to the
# major number of the version
# note that ${public_headers} should be in quotes
set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        RELEASE_POSTFIX "_release"
        DEBUG_POSTFIX "_debug"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        C_VISIBILITY_PRESET "default"
        VISIBILITY_INLINES_HIDDEN TRUE
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME export/export_${PROJECT_NAME}.h
)
target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set(SV_USER_CONFIGURATION ""  CACHE STRING "Freestanding environment with user defined configuration providing necessary functions")
if (SV_USER_CONFIGURATION)
    target_compile_definitions(${PROJECT_NAME} 
        PUBLIC 
        SV_USER_CONFIGURATION_HEADER="${SV_USER_CONFIGURATION}"
    )
endif()
include (cmake/Coverage.cmake)
# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)

