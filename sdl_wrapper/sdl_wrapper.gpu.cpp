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
import :render_data;
import :vertex_layout;
namespace sopho
{
    std::expected<BufferWrapper, GpuError> GpuWrapper::create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size)
    {
        SDL_GPUBufferCreateInfo create_info{.usage = flag, .size = size};
        auto gpu_buffer = SDL_CreateGPUBuffer(device(), &create_info);
        if (!gpu_buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CREATE_GPU_BUFFER_FAILED);
        }
        SDL_GPUTransferBufferCreateInfo transfer_info{};
        transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_info.size = size;
        transfer_info.props = 0;
        auto transfer_buffer = SDL_CreateGPUTransferBuffer(device(), &transfer_info);
        if (!transfer_buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CREATE_TRANSFER_BUFFER_FAILED);
        }
        return BufferWrapper{shared_from_this(), gpu_buffer, transfer_buffer, size};
    }
    std::expected<RenderData, GpuError> GpuWrapper::create_data(const RenderProcedural& render_procedural,
                                                                uint32_t vertex_count)
    {
        auto size = render_procedural.vertex_layout().get_stride() * vertex_count;
        auto buffer = create_buffer(SDL_GPU_BUFFERUSAGE_VERTEX, size);
        if (!buffer)
        {
            return std::unexpected(buffer.error());
        }
        return RenderData{std::move(buffer.value()), render_procedural.vertex_layout()};
    }
    std::expected<RenderProcedural, GpuError> GpuWrapper::create_render_procedural()
    {
        // Query texture format, then construct RenderProcedural
        return get_texture_format().transform([self = shared_from_this()](SDL_GPUTextureFormat format)
                                              { return RenderProcedural{self, format}; });
    }
} // namespace sopho
