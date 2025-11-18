// sdl_wrapper.buffer.cpp
// Created by sophomore on 11/12/25.
//
module;
#include <cstdint>
#include <expected>
#include <memory>
#include <variant>

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_stdinc.h"
module sdl_wrapper;
import :buffer;
import :gpu;

namespace sopho
{
    BufferWrapper::~BufferWrapper() noexcept
    {
        if (!m_gpu)
        {
            return;
        }

        // Release vertex buffer
        if (m_vertex_buffer)
        {
            m_gpu->release_buffer(m_vertex_buffer);
            m_vertex_buffer = nullptr;
        }

        // Release transfer buffer
        if (m_transfer_buffer)
        {
            SDL_ReleaseGPUTransferBuffer(m_gpu->device(), m_transfer_buffer);
            m_transfer_buffer = nullptr;
            m_transfer_buffer_size = 0;
        }
    }

    [[nodiscard]] std::expected<std::monostate, GpuError>
    BufferWrapper::upload()
    {
        auto src_data = m_cpu_buffer.data();
        auto size = m_cpu_buffer.size();
        auto offset = 0;
        // Bounds check to avoid writing past the end of the GPU buffer.
        if (offset + size > m_vertex_buffer_size)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d buffer overflow: size=%u, offset=%u, buffer_size=%u", __FILE__,
                         __LINE__, size, offset, m_vertex_buffer_size);

            return std::unexpected(GpuError::BUFFER_OVERFLOW);
        }

        auto* device = m_gpu->device();

        // 1. Ensure the transfer buffer capacity is sufficient.
        if (size > m_transfer_buffer_size)
        {
            if (m_transfer_buffer != nullptr)
            {
                SDL_ReleaseGPUTransferBuffer(device, m_transfer_buffer);
                m_transfer_buffer = nullptr;
                m_transfer_buffer_size = 0;
            }

            SDL_GPUTransferBufferCreateInfo transfer_info{};
            transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            transfer_info.size = size;
            transfer_info.props = 0;

            m_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
            if (!m_transfer_buffer)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to create transfer buffer: %s", __FILE__, __LINE__,
                             SDL_GetError());

                return std::unexpected(GpuError::CREATE_TRANSFER_BUFFER_FAILED);
            }

            m_transfer_buffer_size = transfer_info.size;
        }

        // 2. Map the transfer buffer and copy data into it.
        void* dst = SDL_MapGPUTransferBuffer(device, m_transfer_buffer, false);
        if (!dst)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to map transfer buffer: %s", __FILE__, __LINE__,
                         SDL_GetError());

            return std::unexpected(GpuError::MAP_TRANSFER_BUFFER_FAILED);
        }

        SDL_memcpy(dst, src_data, size);
        SDL_UnmapGPUTransferBuffer(device, m_transfer_buffer);

        // 3. Acquire a command buffer and enqueue the copy pass.
        auto* command_buffer = SDL_AcquireGPUCommandBuffer(device);
        if (!command_buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to acquire GPU command buffer: %s", __FILE__, __LINE__,
                         SDL_GetError());

            return std::unexpected(GpuError::ACQUIRE_COMMAND_BUFFER_FAILED);
        }

        auto* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

        if (!copy_pass)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to begin GPU copy pass: %s", __FILE__, __LINE__,
                         SDL_GetError());
            SDL_SubmitGPUCommandBuffer(command_buffer);
            return std::unexpected(GpuError::BEGIN_COPY_PASS_FAILED);
        }

        SDL_GPUTransferBufferLocation location{};
        location.transfer_buffer = m_transfer_buffer;
        location.offset = 0;

        SDL_GPUBufferRegion region{};
        region.buffer = m_vertex_buffer;
        region.size = size;
        region.offset = offset;

        SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);

        SDL_EndGPUCopyPass(copy_pass);
        if (!SDL_SubmitGPUCommandBuffer(command_buffer))
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::SUBMIT_COMMAND_FAILED);
        }

        return std::monostate{};
    }

} // namespace sopho
