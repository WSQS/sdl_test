cmake_minimum_required(VERSION 3.20)
project(SDL_TEST LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
include(FetchContent)

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
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

# SPIRV-Headers
FetchContent_Declare(
        SPIRVHeaders
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG vulkan-sdk-1.4.328.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRVHeaders)

# SPIRV-Tools（会用到上面的 Headers）
FetchContent_Declare(
        SPIRVTools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG vulkan-sdk-1.4.328.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        CMAKE_ARGS -DSPIRV-Headers_SOURCE_DIR:PATH=${SPIRVHeaders_SOURCE_DIR}
)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SPIRVTools)

# glslang
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
FetchContent_MakeAvailable(shaderc)

add_executable(SDL_TEST main.cpp)
target_link_libraries(SDL_TEST PRIVATE SDL3::SDL3 shaderc)