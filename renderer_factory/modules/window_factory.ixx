// window_factory.ixx
// Created by sophomore on 12/4/25.
//

export module window_factory;
import data_type;
import window;
import sdl_window;
namespace sopho
{
    export checkable<Window*> create_window(WindowBackend window_backend)
    {
        if (window_backend == WindowBackend::SDL)
        {
            return SDLWindow::create();
        }
    }
} // namespace sopho
