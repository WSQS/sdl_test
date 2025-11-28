// sdl_raii.gpu_buffer.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:gpu_buffer;
namespace sopho
{
    export struct GpuBufferRaii
    {
    private:
        SDL_GPUDevice* m_gpu_device{};
        SDL_GPUBuffer* m_raw{};

    public:
        GpuBufferRaii() noexcept = default;

        explicit GpuBufferRaii(SDL_GPUDevice* device, SDL_GPUBuffer* buffer) noexcept :
            m_gpu_device(device), m_raw(buffer)
        {
        }

        GpuBufferRaii(const GpuBufferRaii&) = delete;
        GpuBufferRaii& operator=(const GpuBufferRaii&) = delete;

        GpuBufferRaii(GpuBufferRaii&& other) noexcept : m_gpu_device(other.m_gpu_device), m_raw(other.m_raw)
        {
            other.m_gpu_device = nullptr;
            other.m_raw = nullptr;
        }

        GpuBufferRaii& operator=(GpuBufferRaii&& other) noexcept
        {
            if (this != &other)
            {
                reset();

                m_gpu_device = other.m_gpu_device;
                m_raw = other.m_raw;

                other.m_gpu_device = nullptr;
                other.m_raw = nullptr;
            }
            return *this;
        }

        ~GpuBufferRaii() noexcept { reset(); }
        void reset(SDL_GPUBuffer* buffer = nullptr, SDL_GPUDevice* device = nullptr) noexcept
        {
            if (m_raw && m_gpu_device)
            {
                SDL_ReleaseGPUBuffer(m_gpu_device, m_raw);
            }

            m_gpu_device = device;
            m_raw = buffer;
        }

        [[nodiscard]] SDL_GPUBuffer* raw() const noexcept { return m_raw; }
        [[nodiscard]] bool valid() const noexcept { return m_gpu_device != nullptr && m_raw != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    };
} // namespace sopho
