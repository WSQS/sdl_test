cmake_minimum_required(VERSION 3.30)
project(renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(renderer STATIC)

target_sources(renderer
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/primitive_renderer.ixx
)
