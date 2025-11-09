cmake_minimum_required(VERSION 3.30)
project(sdl_wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(sdl_wrapper STATIC)

target_sources(sdl_wrapper
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        sdl_wrapper.ixx
        sdl_wrapper.buffer.ixx
        sdl_wrapper.app.ixx
        PRIVATE
        sdl_callback_implement.cpp
)

target_link_libraries(sdl_wrapper PUBLIC SDL3::SDL3)
