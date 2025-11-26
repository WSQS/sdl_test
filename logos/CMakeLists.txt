cmake_minimum_required(VERSION 3.30)
project(logos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(logos STATIC)

target_sources(logos
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/logos.ixx
)
