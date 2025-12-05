cmake_minimum_required(VERSION 3.30)
project(assimp_wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(assimp_wrapper STATIC)

target_sources(assimp_wrapper
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/assimp_wrapper.ixx
        PRIVATE
        src/assimp_wrapper.cpp
)

target_link_libraries(assimp_wrapper PRIVATE assimp data_type renderer)
