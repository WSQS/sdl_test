// sdl_raii.transfer_buffer.ixx
// Created by wsqsy on 11/28/2025.
//

export module sdl_raii:transfer_buffer;
#include "SDL3/SDL_gpu.h"
namespace sopho
{
    export struct TransferBufferRaii
    {
    private:
        SDL_GPUDevice* m_gpu_device{};
        SDL_GPUTransferBuffer* m_transfer_buffer{};

    public:
        TransferBufferRaii() noexcept = default;
        explicit TransferBufferRaii(SDL_GPUDevice* device, SDL_GPUTransferBuffer* buffer) noexcept :
            m_gpu_device(device), m_transfer_buffer(buffer)
        {
        }
        TransferBufferRaii(const TransferBufferRaii&) = delete;
        TransferBufferRaii& operator=(const TransferBufferRaii&) = delete;
        TransferBufferRaii(TransferBufferRaii&& other) noexcept :
            m_gpu_device(other.m_gpu_device), m_transfer_buffer(other.m_transfer_buffer)
        {
            other.m_gpu_device = nullptr;
            other.m_transfer_buffer = nullptr;
        }
        TransferBufferRaii& operator=(TransferBufferRaii&& other) noexcept
        {
            if (this != &other)
            {
                reset();

                m_gpu_device = other.m_gpu_device;
                m_transfer_buffer = other.m_transfer_buffer;

                other.m_gpu_device = nullptr;
                other.m_transfer_buffer = nullptr;
            }
            return *this;
        }
        ~TransferBufferRaii() noexcept { reset(); }
        void reset(SDL_GPUTransferBuffer* buffer = nullptr, SDL_GPUDevice* device = nullptr) noexcept
        {
            if (m_transfer_buffer && m_gpu_device)
            {
                SDL_ReleaseGPUTransferBuffer(m_gpu_device, m_transfer_buffer);
            }

            m_gpu_device = device;

            m_transfer_buffer = buffer;
        }
        [[nodiscard]] SDL_GPUTransferBuffer* raw() const noexcept { return m_transfer_buffer; }
        [[nodiscard]] bool valid() const noexcept { return m_gpu_device != nullptr && m_transfer_buffer != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    };
} // namespace sopho
