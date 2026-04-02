cmake_minimum_required(VERSION 3.15)
project("str_view samples"
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "Testing the str_view library with sample programs."
)
add_custom_target(samples)

include_directories("../str_view")
add_executable(mini_grep mini_grep.c)
target_link_libraries(mini_grep str_view)
add_dependencies(samples mini_grep)

