// sdl_wrapper.gpu.cpp
// Created by wsqsy on 11/14/2025.
//
module;
#include <expected>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
module sdl_wrapper;
import :gpu;
import :render_procedural;
namespace sopho
{
    std::expected<BufferWrapper, GpuError> GpuWrapper::create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size)
    {
        SDL_GPUBufferCreateInfo create_info{.usage = flag, .size = size};
        auto buffer = SDL_CreateGPUBuffer(device(), &create_info);
        if (!buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CREATE_BUFFER_FAILED);
        }
        return BufferWrapper{shared_from_this(), buffer, size};
    }
    std::expected<RenderProcedural, GpuError> GpuWrapper::create_pipeline_wrapper()
    {
        // Query texture format, then construct RenderProcedural
        return get_texture_format().transform([self = shared_from_this()](SDL_GPUTextureFormat format)
                                              { return RenderProcedural{self, format}; });
    }
} // namespace sopho
