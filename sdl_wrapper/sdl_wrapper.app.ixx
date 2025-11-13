//
// Created by sophomore on 11/9/25.
//
module;
#include "SDL3/SDL_init.h"
export module sdl_wrapper:app;
export namespace sopho
{
    class App
    {
    public:
        virtual ~App() = default;

        virtual SDL_AppResult init(int argc, char** argv) = 0;

        virtual SDL_AppResult iterate() = 0;

        virtual SDL_AppResult event(SDL_Event* event) = 0;

        virtual void quit(SDL_AppResult result) = 0;
    };
} // namespace sopho
