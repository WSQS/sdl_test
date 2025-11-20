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
        sdl_wrapper.render_procedural.ixx
        sdl_wrapper.render_data.ixx
        sdl_wrapper.renderable.ixx
        sdl_wrapper.gpu.ixx
        sdl_wrapper.decl.ixx
        sdl_wrapper.vertex_layout.ixx
        PRIVATE
        sdl_wrapper.buffer.cpp
        sdl_wrapper.render_procedural.cpp
        sdl_wrapper.gpu.cpp
        sdl_callback_implement.cpp
)

target_link_libraries(sdl_wrapper PUBLIC SDL3::SDL3 shaderc glsl_reflector)
