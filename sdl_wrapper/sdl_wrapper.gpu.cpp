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
    /**
     * @brief Create a GPU buffer and its associated upload transfer buffer.
     *
     * @param flag Usage flags for the GPU buffer.
     * @param size Size in bytes of the buffer to allocate.
     * @return std::expected<BufferWrapper, GpuError> BufferWrapper containing the created GPU buffer, its transfer
     * buffer, and the buffer size on success; `std::unexpected` with a `GpuError` on failure.
     */
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
    /**
     * @brief Create RenderData for a procedural render setup and given vertex count.
     *
     * Allocates a GPU vertex buffer sized to hold `vertex_count` vertices using the
     * vertex layout from `render_procedural`, and returns a RenderData that owns
     * the allocated buffer, the vertex layout, and the vertex count. If buffer
     * creation fails, returns the corresponding GpuError.
     *
     * @param render_procedural Source procedural that provides the vertex layout.
     * @param vertex_count Number of vertices the allocated buffer must hold.
     * @return RenderData RenderData containing the allocated vertex buffer, the vertex layout, and `vertex_count`.
     */
    std::expected<RenderData, GpuError> GpuWrapper::create_data(const RenderProcedural& render_procedural,
                                                                uint32_t vertex_count)
    {
        auto size = render_procedural.vertex_layout().get_stride() * vertex_count;
        auto vertex_buffer = create_buffer(SDL_GPU_BUFFERUSAGE_VERTEX, size);
        auto index_buffer = create_buffer(SDL_GPU_BUFFERUSAGE_INDEX, 6 * sizeof(int));
        if (!vertex_buffer)
        {
            return std::unexpected(vertex_buffer.error());
        }
        if (!index_buffer)
        {
            return std::unexpected(index_buffer.error());
        }
        return RenderData{std::move(vertex_buffer.value()), std::move(index_buffer.value()),
                          render_procedural.vertex_layout(), vertex_count};
    }
    /**
     * @brief Create a RenderProcedural configured for the device's texture format.
     *
     * Queries the GPU's texture format and constructs a RenderProcedural associated with this GpuWrapper.
     *
     * @return std::expected<RenderProcedural, GpuError> Contains the constructed RenderProcedural on success, or an
     * unexpected holding the corresponding GpuError on failure.
     */
    std::expected<RenderProcedural, GpuError> GpuWrapper::create_render_procedural()
    {
        // Query texture format, then construct RenderProcedural
        return get_texture_format().transform([self = shared_from_this()](SDL_GPUTextureFormat format)
                                              { return RenderProcedural{self, format}; });
    }
} // namespace sopho
