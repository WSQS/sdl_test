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
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.26
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
        SPIRVHeaders
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG vulkan-sdk-1.4.328.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRVHeaders)

FetchContent_Declare(
        SPIRVTools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG vulkan-sdk-1.4.328.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SPIRVTools)

FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG vulkan-sdk-1.4.328.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_JS OFF CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glslang)

FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2025.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(shaderc)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
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
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
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
