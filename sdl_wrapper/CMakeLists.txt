cmake_minimum_required(VERSION 3.30)
project(sdl_wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sdl_wrapper STATIC)

# Implementation sources
set(SDL_WRAPPER_IMPL_SOURCES
    sdl_wrapper.buffer.cpp
    sdl_callback_implement.cpp
)

# C++ module interface sources
set(SDL_WRAPPER_MODULE_IFACES
    sdl_wrapper.ixx
    sdl_wrapper.buffer.ixx
    sdl_wrapper.app.ixx
    sdl_wrapper.pipeline.ixx
    sdl_wrapper.gpu.ixx
    sdl_wrapper.decl.ixx
)

# Detect if the current generator supports C++ modules.
# Modules are only supported by:
# - Ninja
# - Ninja Multi-Config
# - Visual Studio 17.4+ generators
set(_GEN "${CMAKE_GENERATOR}")
if(_GEN MATCHES "Ninja" OR _GEN MATCHES "Visual Studio")
    # Generator supports modules: use proper CXX_MODULES file set.
    target_sources(sdl_wrapper
        PUBLIC
            FILE_SET cxx_modules TYPE CXX_MODULES FILES
                ${SDL_WRAPPER_MODULE_IFACES}
        PRIVATE
            ${SDL_WRAPPER_IMPL_SOURCES}
    )

    # Explicitly enable module scanning for this target.
    set_target_properties(sdl_wrapper PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
else()
    # Fallback: generator (e.g. Unix Makefiles) does not support modules.
    # Treat .ixx files as normal sources so configuration and compilation still succeed.
    message(STATUS
        "C++ modules are not supported by generator '${CMAKE_GENERATOR}'. "
        "Building 'sdl_wrapper' without CMake module support (using .ixx as regular sources)."
    )

    target_sources(sdl_wrapper
        PUBLIC
            ${SDL_WRAPPER_MODULE_IFACES}
        PRIVATE
            ${SDL_WRAPPER_IMPL_SOURCES}
    )
endif()

target_link_libraries(sdl_wrapper PUBLIC SDL3::SDL3)
