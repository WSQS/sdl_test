// sdl_raii.gpu_device.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:gpu_device;
namespace sopho
{
    export struct GpuDeviceRaii
    {
    private:
        SDL_GPUDevice* m_raw{};

    public:
        GpuDeviceRaii() noexcept = default;
        explicit GpuDeviceRaii(SDL_GPUDevice* d) noexcept : m_raw(d) {}

        GpuDeviceRaii(GpuDeviceRaii&& other) noexcept : m_raw(other.m_raw) { other.m_raw = nullptr; }
        GpuDeviceRaii& operator=(GpuDeviceRaii&& other) noexcept
        {
            if (this != &other)
            {
                reset(other.m_raw);
                other.m_raw = nullptr;
            }
            return *this;
        }

        GpuDeviceRaii(const GpuDeviceRaii&) = delete;
        GpuDeviceRaii& operator=(const GpuDeviceRaii&) = delete;

        ~GpuDeviceRaii() noexcept { reset(); }

        void reset(SDL_GPUDevice* d = nullptr) noexcept
        {
            if (m_raw != d)
            {
                if (m_raw)
                    SDL_DestroyGPUDevice(m_raw);
                m_raw = d;
            }
        }
        [[nodiscard]] SDL_GPUDevice* raw() const noexcept { return m_raw; }
        [[nodiscard]] bool valid() const noexcept { return m_raw != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return m_raw != nullptr; }
    };
} // namespace sopho
