cmake_minimum_required(VERSION 3.30)
project(sdl_wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_library(sdl_wrapper STATIC)

target_sources(sdl_wrapper
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        modules/lifecycle.ixx
        modules/sdl_raii/sdl_raii.ixx
        modules/sdl_raii/sdl_raii.gpu_device.ixx
        modules/sdl_raii/sdl_raii.window.ixx
        modules/sdl_raii/sdl_raii.claim_window.ixx
        modules/sdl_raii/sdl_raii.transfer_buffer.ixx
        modules/sdl_raii/sdl_raii.gpu_buffer.ixx
        modules/sdl_wrapper.ixx
        modules/sdl_wrapper.buffer.ixx
        modules/sdl_wrapper.render_procedural.ixx
        modules/sdl_wrapper.render_data.ixx
        modules/sdl_wrapper.render_data_impl.ixx
        modules/sdl_wrapper.renderable.ixx
        modules/sdl_wrapper.gpu.ixx
        modules/sdl_wrapper.decl.ixx
        modules/sdl_wrapper.vertex_layout.ixx
        modules/sdl_wrapper.transfer_buffer.ixx
        modules/sdl_wrapper.texture.ixx
        PRIVATE
        src/lifecycle.cpp
        src/sdl_wrapper.buffer.cpp
        src/sdl_wrapper.render_procedural.cpp
        src/sdl_wrapper.gpu.cpp
        src/sdl_wrapper.texture.cpp
        src/sdl_wrapper.transfer_buffer.cpp
        src/sdl_wrapper.render_data.cpp
)

target_link_libraries(sdl_wrapper PUBLIC SDL3::SDL3 shaderc glsl_reflector data_type)
