cmake_minimum_required(VERSION 3.30)
project(sdl_lifecycle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(sdl_lifecycle STATIC)

target_sources(sdl_lifecycle
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/sdl_lifecycle.ixx
        PRIVATE
        src/lifecycle.cpp
)

target_link_libraries(sdl_lifecycle PRIVATE SDL3::SDL3 data_type)
