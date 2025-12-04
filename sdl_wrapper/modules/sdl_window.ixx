// sdl_window.ixx
// Created by wsqsy on 12/4/2025.
//

export module sdl_window;
import data_type;
import window;
import sdl_raii;
import <SDL3/SDL_video.h>;
import <cstdint>;
namespace sopho
{
    class SDLWindow : public Window
    {
        WindowRaii window_raii{};

    public:
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
