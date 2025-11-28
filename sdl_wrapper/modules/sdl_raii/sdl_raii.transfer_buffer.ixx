// sdl_raii.transfer_buffer.ixx
// Created by wsqsy on 11/28/2025.
//

export module sdl_raii:transfer_buffer;
#include <SDL3/SDL_gpu.h>
namespace sopho
{
    export struct TransferBufferRaii
    {
    private:
        SDL_GPUDevice* m_gpu_device{};
        SDL_GPUTransferBuffer* m_raw{};

    public:
        TransferBufferRaii() noexcept = default;
        TransferBufferRaii(SDL_GPUDevice* device, SDL_GPUTransferBuffer* buffer) noexcept :
            m_gpu_device(device), m_raw(buffer)
        {
        }
        TransferBufferRaii(const TransferBufferRaii&) = delete;
        TransferBufferRaii& operator=(const TransferBufferRaii&) = delete;
        TransferBufferRaii(TransferBufferRaii&& other) noexcept :
            m_gpu_device(other.m_gpu_device), m_raw(other.m_raw)
        {
            other.m_gpu_device = nullptr;
            other.m_raw = nullptr;
        }
        TransferBufferRaii& operator=(TransferBufferRaii&& other) noexcept
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
        ~TransferBufferRaii() noexcept { reset(); }
        void reset(SDL_GPUTransferBuffer* buffer = nullptr, SDL_GPUDevice* device = nullptr) noexcept
        {
            if (m_raw && m_gpu_device)
            {
                SDL_ReleaseGPUTransferBuffer(m_gpu_device, m_raw);
            }

            m_gpu_device = device;

            m_raw = buffer;
        }
        [[nodiscard]] SDL_GPUTransferBuffer* raw() const noexcept { return m_raw; }
        [[nodiscard]] bool valid() const noexcept { return m_gpu_device != nullptr && m_raw != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    };
} // namespace sopho
