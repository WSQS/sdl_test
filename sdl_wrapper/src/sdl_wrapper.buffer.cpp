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

        auto rst = m_transfer_buffer.submit(src_data);
        if (!rst)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to map transfer buffer: %s", __FILE__, __LINE__,
                         SDL_GetError());
            return std::unexpected(rst.error());
        }
        GpuCommandBufferRaii command_buffer_raii;
        {
            // 3. Acquire a command buffer and enqueue the copy pass.
            auto* command_buffer = SDL_AcquireGPUCommandBuffer(device);
            if (!command_buffer)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to acquire GPU command buffer: %s", __FILE__, __LINE__,
                             SDL_GetError());

                return std::unexpected(GpuError::ACQUIRE_COMMAND_BUFFER_FAILED);
            }
            command_buffer_raii.reset(command_buffer);
        }

        auto* copy_pass = SDL_BeginGPUCopyPass(command_buffer_raii.raw());

        if (!copy_pass)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to begin GPU copy pass: %s", __FILE__, __LINE__,
                         SDL_GetError());
            return std::unexpected(GpuError::BEGIN_COPY_PASS_FAILED);
        }

        SDL_GPUTransferBufferLocation location{};
        location.transfer_buffer = m_transfer_buffer.raw();
        location.offset = 0;

        SDL_GPUBufferRegion region{};
        region.buffer = m_gpu_buffer.raw();
        region.size = size;
        region.offset = 0;

        SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);

        SDL_EndGPUCopyPass(copy_pass);

        return std::monostate{};
    }

    checkable<BufferWrapper> BufferWrapper::Builder::build(GpuWrapper& gpu)
    {
        SDL_GPUBufferCreateInfo create_info{.usage = flag, .size = size};
        auto gpu_buffer = SDL_CreateGPUBuffer(gpu.device(), &create_info);
        if (!gpu_buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CREATE_GPU_BUFFER_FAILED);
        }
        GpuBufferRaii gpu_buffer_raii{gpu.device(), gpu_buffer};
        auto transfer_buffer = TransferBufferWrapper::Builder{}
                                   .set_size(size)
                                   .set_usage(SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD)
                                   .set_usage_limit(-1)
                                   .build(gpu);
        if (!transfer_buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(transfer_buffer.error());
        }
        return BufferWrapper{gpu.shared_from_this(), std::move(gpu_buffer_raii), (std::move(transfer_buffer.value())),
                             size};
    }

} // namespace sopho
