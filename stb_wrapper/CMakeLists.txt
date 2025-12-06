cmake_minimum_required(VERSION 3.30)
project(stb_wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(stb_wrapper STATIC)

target_sources(stb_wrapper
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/stb_wrapper.ixx
        PRIVATE
        src/stb_wrapper.cpp
)

target_link_libraries(stb_wrapper PRIVATE stb data_type SDL3::SDL3)
