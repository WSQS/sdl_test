// window.ixx
// Created by wsqsy on 12/4/2025.
//
export module window;
import data_type;
import <cstdint>;
export namespace sopho
{
    struct WindowSize
    {
        std::uint32_t width{};
        std::uint32_t height{};
    };
    struct NativeWindowHandle
    {
        WindowBackend window_backend{};
        void* ptr{};
    };

    class Window
    {
    public:
        virtual ~Window() = default;
        virtual WindowSize size() = 0;
        virtual NativeWindowHandle native_handle() = 0;
    };
} // namespace sopho
