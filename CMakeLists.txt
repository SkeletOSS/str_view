# File sets are available in 3.23 onward. Valuable tool.
cmake_minimum_required(VERSION 3.23)

set(namespace "str_view")
project("str_view"
    VERSION 0.7.3
    LANGUAGES C
    DESCRIPTION "Robust read-only string handling, tokenization, and matching in C."
)

include_directories("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}")
add_subdirectory("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}")

# For the sake of vcpkg we will make sure cloning and building from
# main will only build the minimal files in the all target. I can't
# protect vcpkg from needlessly downloading 
if (EXISTS "${PROJECT_SOURCE_DIR}/samples")
    add_subdirectory("${PROJECT_SOURCE_DIR}/samples" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${PROJECT_SOURCE_DIR}/tests")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tests" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${PROJECT_SOURCE_DIR}/tests" AND EXISTS "${PROJECT_SOURCE_DIR}/samples")
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

# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)

set(SV_COVERAGE_TOOL "" CACHE STRING "Code coverage tool to use for coverage report")
# Code coverage is easy on gcc but clang requires some extra steps. We can 
# indicate via our presets that we are using a code coverage preset and we can
# also manually specify the code coverage tool if we have one installed. This
# is helpful for gcc, for example, because I have a newer gcov I like to use
# for the gcc-XX.X compiler; this is not the default one installed.
if (SV_ENABLE_COVERAGE)

    if (SV_COVERAGE_TOOL)
        set(COVERAGE_TOOL ${SV_COVERAGE_TOOL})
        message(STATUS "Using code coverage tool: ${SV_COVERAGE_TOOL}")
    endif()

    # Clang had very tricky behavior especially on mac to successfully generate
    # lcov compatible reports. The command line would not accept clang's 
    # provided 'llvm-cov gcov' string so a custom script that executes that 
    # command needs to be generated. We will just place it in the build files.
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        if (COVERAGE_TOOL)
            set(LLVM_COV_EXECUTABLE ${COVERAGE_TOOL})
        else()
            find_program(LLVM_COV_EXECUTABLE llvm-cov)
            if (NOT LLVM_COV_EXECUTABLE)
                message(FATAL_ERROR "llvm-cov not found")
            endif()
        endif()

        set(LLVM_GCOV_WRAPPER "${CMAKE_BINARY_DIR}/llvm-gcov")

        file(WRITE ${LLVM_GCOV_WRAPPER}
"#!/usr/bin/env bash
exec \"${LLVM_COV_EXECUTABLE}\" gcov \"$@\"
")

        file(CHMOD ${LLVM_GCOV_WRAPPER}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE)

        set(COVERAGE_TOOL ${LLVM_GCOV_WRAPPER})
    endif()

    if (NOT COVERAGE_TOOL)
        message(FATAL_ERROR "no code coverage tool selected")
    endif()

    # Developer coverage reports include all test files to confirm which test
    # cod is ran. This should be for the local developer machine.
    add_custom_target(coverage-developer
        COMMAND lcov --ignore-errors inconsistent 
                     --gcov-tool ${COVERAGE_TOOL}
                     --capture
                     --directory ${CMAKE_BINARY_DIR}
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND lcov --ignore-errors inconsistent
                     --extract ${CMAKE_SOURCE_DIR}/docs/coverage.info 
                     '${CMAKE_SOURCE_DIR}/tests/*' 
                     '${CMAKE_SOURCE_DIR}/source/*'
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND lcov --remove ${CMAKE_SOURCE_DIR}/docs/coverage.info 
                     '${CMAKE_SOURCE_DIR}/tests/run_tests.c' 
                     -o ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND genhtml ${CMAKE_SOURCE_DIR}/docs/coverage.info
                        --output-directory ${CMAKE_SOURCE_DIR}/docs/coverage
                        --prefix ${CMAKE_SOURCE_DIR}
                        --title "SV Test Suite Coverage Report"
    )
    
    # The published report on github pages should not include tests as I don't
    # want the C Container Collection to be seen as including tests to inflate
    # coverage numbers. We want to build trust and show we are working hard on
    # good library not just padding imaginary numbers.
    add_custom_target(coverage-publish
        COMMAND lcov --ignore-errors inconsistent 
                     --gcov-tool ${COVERAGE_TOOL}
                     --capture
                     --directory ${CMAKE_BINARY_DIR}
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND lcov --ignore-errors inconsistent
                     --extract ${CMAKE_SOURCE_DIR}/docs/coverage.info 
                     '${CMAKE_SOURCE_DIR}/source/*'
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND genhtml ${CMAKE_SOURCE_DIR}/docs/coverage.info
                        --output-directory ${CMAKE_SOURCE_DIR}/docs/coverage
                        --prefix ${CMAKE_SOURCE_DIR}
                        --flat
                        --title "SV Test Suite Coverage Report"
    )
endif()
