cmake_minimum_required(VERSION 3.15)
project("str_view tests"
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "Testing the str_view library."
)
add_custom_target(tests)

add_library(test INTERFACE test.h)
add_dependencies(tests test)
include_directories("../str_view")
add_executable(run_tests run_tests.c)
add_dependencies(tests run_tests)
target_link_libraries(run_tests str_view test)

###########   Add Test Files Below This Point   ###########

# TEST_NAME should be the name of the .c file added to the test folder
# and the macro will add the target, link the libraries, and put the
# the test executable in the tests folder either in debug or release
# depending on the build type. The run_tests program then runs all binaries.
macro(add_test TEST_NAME)
  add_executable(${TEST_NAME} ${TEST_NAME}.c)
  target_link_libraries(${TEST_NAME} 
    str_view 
    test
  )
  set_target_properties(${TEST_NAME} 
    PROPERTIES 
      RUNTIME_OUTPUT_DIRECTORY 
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
  )
  add_dependencies(tests ${TEST_NAME})
endmacro()

# Add tests below here by the name of the c file without the .c suffix
add_test(test_empty)
add_test(test_out_of_bounds)
add_test(test_constructors)
add_test(test_str_positions)
add_test(test_copy_fill)
add_test(test_length_vs_bytes)
add_test(test_iterators)
add_test(test_comparisons)
add_test(test_string_searching)
add_test(test_view_editing)

