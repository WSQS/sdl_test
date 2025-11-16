// sdl_wrapper.gpu.cpp
// Created by wsqsy on 11/14/2025.
//
module;
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
module sdl_wrapper;
import :gpu;
import :pipeline;
namespace sopho
{
    std::expected<BufferWrapper, GpuError> GpuWrapper::create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size)
    {
        return m_device.and_then(
            [&](auto device) -> std::expected<BufferWrapper, GpuError>
            {
                SDL_GPUBufferCreateInfo create_info{flag, size};
                auto buffer = SDL_CreateGPUBuffer(device, &create_info);
                if (!buffer)
                {
                    SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                    return std::unexpected(GpuError::CREATE_BUFFER_FAILED);
                }
                return BufferWrapper{shared_from_this(), buffer};
            });
    }
    std::expected<PipelineWrapper,GpuError> GpuWrapper::create_pipeline() {
        return m_device.transform([&](auto device)-> PipelineWrapper {
            return PipelineWrapper{shared_from_this()};
        });
    }
} // namespace sopho
