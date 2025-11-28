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
    void TransferBufferWrapper::reset() noexcept
    {
        if (m_gpu && m_gpu->device())
        {
            m_transfer_buffer.reset();
        }
    }

    checkable<std::monostate> TransferBufferWrapper::submit(const void* data_source)
    {
        assert(m_usage_limit != 0);
        void* dst = SDL_MapGPUTransferBuffer(m_gpu->device(), raw(), false);
        if (!dst)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to map transfer buffer: %s", __FILE__, __LINE__,
                         SDL_GetError());

            return std::unexpected(GpuError::MAP_TRANSFER_BUFFER_FAILED);
        }

        SDL_memcpy(dst, data_source, m_size);
        SDL_UnmapGPUTransferBuffer(m_gpu->device(), raw());
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
        TransferBufferRaii tb_raii{gpu.device(), transfer_buffer};

        return TransferBufferWrapper{gpu.shared_from_this(), std::move(tb_raii), usage_limit, size};
    }
} // namespace sopho
