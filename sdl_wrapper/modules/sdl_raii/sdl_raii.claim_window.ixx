// sdl_raii.claim_window.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:claim_window;
namespace sopho
{
    export struct ClaimWindowRaii
    {
    private:
        SDL_GPUDevice* m_gpu_device{};
        SDL_Window* m_window{};

    public:
        ClaimWindowRaii() noexcept = default;

        ClaimWindowRaii(SDL_GPUDevice* d, SDL_Window* w) noexcept : m_gpu_device(d), m_window(w) {}

        ClaimWindowRaii(const ClaimWindowRaii&) = delete;
        ClaimWindowRaii& operator=(const ClaimWindowRaii&) = delete;

        ClaimWindowRaii(ClaimWindowRaii&& other) noexcept : m_gpu_device(other.m_gpu_device), m_window(other.m_window)
        {
            other.m_gpu_device = nullptr;
            other.m_window = nullptr;
        }

        ClaimWindowRaii& operator=(ClaimWindowRaii&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                m_gpu_device = other.m_gpu_device;
                m_window = other.m_window;
                other.m_gpu_device = nullptr;
                other.m_window = nullptr;
            }
            return *this;
        }

        ~ClaimWindowRaii() noexcept { reset(); }

        void reset(SDL_GPUDevice* d = nullptr, SDL_Window* w = nullptr) noexcept
        {
            if (m_gpu_device != d || m_window != w)
            {
                if (m_gpu_device && m_window)
                    SDL_ReleaseWindowFromGPUDevice(m_gpu_device, m_window);

                m_gpu_device = d;
                m_window = w;
            }
        }

        [[nodiscard]] bool valid() const noexcept { return m_gpu_device != nullptr && m_window != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    };
} // namespace sopho
