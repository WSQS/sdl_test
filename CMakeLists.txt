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

add_executable(SDL_TEST main.cpp)
target_link_libraries(SDL_TEST PRIVATE SDL3::SDL3)