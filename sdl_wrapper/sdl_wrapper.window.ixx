//
// Created by wsqsy on 11/14/2025.
//
module;
#include <memory>
#include "SDL3/SDL_video.h"
export module sdl_wrapper:window;
import :decl;
export namespace sopho
{
    class WindowWrapper
    {
        SDL_Window* m_window{};
        std::shared_ptr<GpuWrapper> m_gpu;
    public:
        WindowWrapper(std::shared_ptr<GpuWrapper> p_gpu, SDL_Window* p_window);
        ~WindowWrapper();
        auto data()
        {
            return m_window;
        }
        friend GpuWrapper;
    };
} // namespace sopho
