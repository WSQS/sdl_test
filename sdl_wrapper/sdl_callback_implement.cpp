//
// Created by sophomore on 11/9/25.
//
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
import sdl_wrapper;


extern sopho::App *create_app();

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    auto app = create_app();
    *appstate = app;
    return app->init(argc, argv);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto app = static_cast<sopho::App *>(appstate);
    return app->iterate();
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto app = static_cast<sopho::App *>(appstate);
    return app->event(event);
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto app = static_cast<sopho::App *>(appstate);
    app->quit(result);
    delete app;
}
