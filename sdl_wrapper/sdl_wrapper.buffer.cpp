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
    /**
     * @brief Releases any owned GPU resources held by the wrapper.
     *
     * Ensures the associated GPU object is valid, then releases the GPU buffer and the
     * GPU transfer buffer if they exist and clears their pointers.
     */
    BufferWrapper::~BufferWrapper() noexcept
    {
        if (!m_gpu)
        {
            return;
        }

        // Release gpu buffer
        if (m_gpu_buffer)
        {
            m_gpu->release_buffer(m_gpu_buffer);
            m_gpu_buffer = nullptr;
        }

        // Release transfer buffer
        if (m_transfer_buffer)
        {
            SDL_ReleaseGPUTransferBuffer(m_gpu->device(), m_transfer_buffer);
            m_transfer_buffer = nullptr;
        }
    }

    /**
     * @brief Uploads the internal CPU-side buffer to the GPU buffer via the transfer buffer and a GPU copy pass.
     *
     * Copies the contents of m_cpu_buffer into the existing transfer buffer, records a GPU copy pass that
     * uploads that transfer buffer into m_gpu_buffer, submits the command buffer, and returns success status.
     *
     * @returns std::monostate on success; otherwise an unexpected `GpuError` indicating the failure:
     *  - `GpuError::MAP_TRANSFER_BUFFER_FAILED` if mapping the transfer buffer failed.
     *  - `GpuError::ACQUIRE_COMMAND_BUFFER_FAILED` if acquiring a GPU command buffer failed.
     *  - `GpuError::BEGIN_COPY_PASS_FAILED` if beginning the GPU copy pass failed.
     *  - `GpuError::SUBMIT_COMMAND_FAILED` if submitting the GPU command buffer failed.
     */
    [[nodiscard]] std::expected<std::monostate, GpuError> BufferWrapper::upload()
    {
        auto src_data = m_cpu_buffer.data();
        auto size = m_cpu_buffer.size();

        auto* device = m_gpu->device();

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
        region.buffer = m_gpu_buffer;
        region.size = size;
        region.offset = 0;

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
