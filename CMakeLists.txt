cmake_minimum_required(VERSION 3.30)
project(SDL_TEST LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FETCHCONTENT_QUIET OFF CACHE BOOL "" FORCE)
set(CMAKE_DOWNLOAD_NO_PROGRESS OFF CACHE BOOL "" FORCE)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS CACHE STRING "" FORCE)

FetchContent_Declare(
        SDL3
        URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.26.zip
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
        SPIRVHeaders
        URL https://github.com/KhronosGroup/SPIRV-Headers/archive/refs/tags/vulkan-sdk-1.4.328.1.zip
)
FetchContent_MakeAvailable(SPIRVHeaders)

FetchContent_Declare(
        SPIRVTools
        URL https://github.com/KhronosGroup/SPIRV-Tools/archive/refs/tags/vulkan-sdk-1.4.328.1.zip
)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SPIRVTools)

FetchContent_Declare(
        glslang
        URL https://github.com/KhronosGroup/glslang/archive/refs/tags/vulkan-sdk-1.4.328.1.zip
)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_JS OFF CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glslang)

FetchContent_Declare(
        shaderc
        URL https://github.com/google/shaderc/archive/refs/tags/v2025.4.zip
)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(shaderc)

FetchContent_Declare(
        imgui
        URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.4.zip
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlgpu3.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui SYSTEM PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL3::SDL3)

FetchContent_Declare(
        stb
        URL https://github.com/nothings/stb/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
include(GoogleTest)

add_subdirectory(data_type)
add_subdirectory(sdl_wrapper)
add_subdirectory(glsl_reflector)
add_subdirectory(logos)

add_executable(SDL_TEST main.cpp)
target_link_libraries(SDL_TEST PRIVATE imgui sdl_wrapper stb logos)

include(GNUInstallDirs)
install(TARGETS SDL_TEST
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(ASSETS_DST_DIR ${CMAKE_BINARY_DIR}/assets)
file(MAKE_DIRECTORY ${ASSETS_DST_DIR})

set(TEST_TEXTURE_PATH ${ASSETS_DST_DIR}/test_texture.png)

if (NOT EXISTS ${TEST_TEXTURE_PATH})
    file(DOWNLOAD
            "https://opengameart.org/sites/default/files/slime_25.png"
            "${TEST_TEXTURE_PATH}"
            SHOW_PROGRESS
    )
endif()

install(DIRECTORY ${ASSETS_DST_DIR}/
        DESTINATION ${CMAKE_INSTALL_BINDIR}/assets
)
