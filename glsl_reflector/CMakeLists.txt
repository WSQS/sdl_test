cmake_minimum_required(VERSION 3.30)
project(glsl_reflector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(glsl_reflector STATIC)

target_sources(glsl_reflector
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/glsl_reflector.ixx
        modules/glsl_reflector.type.ixx
)

target_link_libraries(glsl_reflector PUBLIC glslang glslang::glslang-default-resource-limits)

add_executable(glslr test/test.cpp)

target_link_libraries(glslr PUBLIC glsl_reflector gtest gtest_main)

add_test(NAME unit_tests COMMAND glslr)
gtest_discover_tests(glslr)
