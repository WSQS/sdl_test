cmake_minimum_required(VERSION 3.30)
project(sdl_gl_primitive_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(sdl_gl_primitive_renderer STATIC)

target_sources(sdl_gl_primitive_renderer
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/sdl_gl_primitive_renderer.ixx
)

target_link_libraries(sdl_gl_primitive_renderer PRIVATE SDL3::SDL3 renderer glad_gl_core_46)
