//
// Created by sophomore on 11/9/25.
//
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
import sdl_wrapper;

namespace sopho
{
    extern App* create_app(int argc, char** argv);
}

/**
 * @brief Initializes the SDL video subsystem, constructs the application, and invokes its initialization.
 *
 * @param appstate Pointer to storage that will receive the created sopho::App* on success.
 * @param argc Program argument count forwarded to the application's init.
 * @param argv Program argument vector forwarded to the application's init.
 * @return SDL_AppResult Result returned by the application's init, or SDL_APP_FAILURE if application creation failed.
 */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    auto app = sopho::create_app(argc, argv);
    if (!app)
        return SDL_APP_FAILURE;
    *appstate = app;
    return app->init(argc, argv);
}

/**
 * @brief Run a single per-frame iteration on the application.
 *
 * @param appstate Pointer to the sopho::App instance stored by SDL_AppInit.
 * @param result  Result code describing why the application is quitting.
 * @return SDL_AppResult The application's requested next action.
 */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto* app = static_cast<sopho::App*>(appstate);
    return app->iterate();
}

/**
 * @brief Dispatches an SDL event to the stored application instance.
 *
 * @param appstate Opaque pointer previously set by SDL_AppInit; must point to a `sopho::App` instance.
 * @param event Pointer to the SDL event to deliver to the application.
 * @return SDL_AppResult Result returned by the application's event handler.
 */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    auto* app = static_cast<sopho::App*>(appstate);
    return app->event(event);
}

/**
 * @brief Shuts down the application and releases its instance.
 *
 * Invokes the application's quit handler with the provided result and destroys
 * the stored application instance.
 *
 * @param appstate Pointer to the sopho::App instance previously stored by SDL_AppInit.
 * @param result  Result code describing why the application is quitting.
 */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto* app = static_cast<sopho::App*>(appstate);
    app->quit(result);
    delete app;
}
