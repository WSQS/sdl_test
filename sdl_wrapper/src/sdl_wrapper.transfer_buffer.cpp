// sdl_wrapper.transfer_buffer.cpp
// Created by sophomore on 11/25/25.
//
module;
#include <cassert>
#include <expected>
#include <utility>
#include <variant>

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
module sdl_wrapper;
import :transfer_buffer;
import :gpu;
namespace sopho
{
    TransferBufferWrapper& TransferBufferWrapper::operator=(TransferBufferWrapper&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            m_gpu = std::move(other.m_gpu);
            m_transfer_buffer = other.m_transfer_buffer;
            m_usage_limit = other.m_usage_limit;
            m_size = other.m_size;
            other.m_transfer_buffer = nullptr;
        }
        return *this;
    }
    void TransferBufferWrapper::reset() noexcept
    {
        if (m_transfer_buffer && m_gpu && m_gpu->device())
        {
            SDL_ReleaseGPUTransferBuffer(m_gpu->device(), m_transfer_buffer);
            m_transfer_buffer = nullptr;
        }
    }

    checkable<std::monostate> TransferBufferWrapper::submit(void* data_source)
    {
        assert(m_usage_limit != 0);
        void* dst = SDL_MapGPUTransferBuffer(m_gpu->device(), m_transfer_buffer, false);
        if (!dst)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to map transfer buffer: %s", __FILE__, __LINE__,
                         SDL_GetError());

            return std::unexpected(GpuError::MAP_TRANSFER_BUFFER_FAILED);
        }

        SDL_memcpy(dst, data_source, m_size);
        SDL_UnmapGPUTransferBuffer(m_gpu->device(), m_transfer_buffer);
        if (m_usage_limit != -1)
        {
            m_usage_limit--;
            if (m_usage_limit == 0)
            {
                reset();
            }
        }
        return {};
    }
    checkable<TransferBufferWrapper> TransferBufferWrapper::Builder::build(GpuWrapper& gpu)
    {
        SDL_GPUTransferBufferCreateInfo create_info{};
        create_info.usage = usage;
        create_info.size = size;

        auto* transfer_buffer = SDL_CreateGPUTransferBuffer(gpu.device(), &create_info);
        if (!transfer_buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CREATE_TRANSFER_BUFFER_FAILED);
        }

        return TransferBufferWrapper{gpu.shared_from_this(), transfer_buffer, usage_limit, size};
    }
} // namespace sopho
