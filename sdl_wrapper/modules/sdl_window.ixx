// sdl_window.ixx
// Created by wsqsy on 12/4/2025.
//
module;
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <expected>
#include <utility>
export module sdl_window;
import data_type;
import window;
import sdl_raii;
namespace sopho
{
    export class SDLWindow : public Window
    {
        WindowRaii window_raii{};

    public:
        explicit SDLWindow(WindowRaii window_raii) : window_raii(std::move(window_raii)) {};
        static checkable<Window*> create()
        {
            auto win_raw = SDL_CreateWindow("Hello world!", 960, 540, SDL_WINDOW_RESIZABLE);
            if (!win_raw)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                return std::unexpected(GpuError::CREATE_WINDOW_FAILED);
            }
            WindowRaii window{win_raw};
            return new SDLWindow(std::move(window));
        }
        WindowSize size() override
        {
            int x{}, y{};
            SDL_GetWindowSize(window_raii.raw(), &x, &y);
            return {static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y)};
        }
        NativeWindowHandle native_handle() override
        {
            return NativeWindowHandle{WindowBackend::SDL, window_raii.raw()};
        }
    };
} // namespace sopho
