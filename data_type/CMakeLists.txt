cmake_minimum_required(VERSION 3.30)
project(data_type LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(data_type STATIC)

target_sources(data_type
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/data_type.ixx
)
