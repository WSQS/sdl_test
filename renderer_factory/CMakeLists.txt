cmake_minimum_required(VERSION 3.30)
project(renderer_factory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(renderer_factory STATIC)

target_sources(renderer_factory
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/renderer_factory.ixx
)

target_link_libraries(renderer_factory PRIVATE data_type renderer sdl_wrapper sdl_gl_primitive_renderer)
