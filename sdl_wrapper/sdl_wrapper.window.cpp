//
// Created by wsqsy on 11/14/2025.
//
module;
module sdl_wrapper;
import :window;
import :gpu;

namespace sopho
{

    WindowWrapper::WindowWrapper(std::shared_ptr<GpuWrapper> p_gpu, SDL_Window* p_window)
        :m_gpu(p_gpu),m_window(p_window)
    {
    }
    WindowWrapper::~WindowWrapper()
    {

    }

} // namespace sopho
