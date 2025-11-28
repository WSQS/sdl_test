// sdl_raii.window.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:window;
namespace sopho
{
    export struct WindowRaii
    {
    private:
        SDL_Window* m_raw{};

    public:
        WindowRaii() = default;

        explicit WindowRaii(SDL_Window* w) noexcept : m_raw(w) {}

        WindowRaii(const WindowRaii&) = delete;
        WindowRaii& operator=(const WindowRaii&) = delete;

        WindowRaii(WindowRaii&& other) noexcept : m_raw(other.m_raw) { other.m_raw = nullptr; }

        WindowRaii& operator=(WindowRaii&& other) noexcept
        {
            if (this != &other)
            {
                reset(other.m_raw);
                other.m_raw = nullptr;
            }
            return *this;
        }

        ~WindowRaii() noexcept { reset(); }

        void reset(SDL_Window* w = nullptr) noexcept
        {
            if (m_raw != w)
            {
                if (m_raw)
                    SDL_DestroyWindow(m_raw);
                m_raw = w;
            }
        }

        [[nodiscard]] SDL_Window* raw() const noexcept { return m_raw; }
        [[nodiscard]] bool valid() const noexcept { return m_raw != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return m_raw != nullptr; }
    };
} // namespace sopho
