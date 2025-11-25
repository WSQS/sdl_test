// sdl_wrapper.gpu.cpp
// Created by wsqsy on 11/14/2025.
//
module;
#include <expected>
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
module sdl_wrapper;
import :gpu;
import :render_procedural;
import :buffer;
import :render_data;
import :vertex_layout;
import :transfer_buffer;
import :render_data_impl;
namespace sopho
{

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
    checkable<std::shared_ptr<RenderData>> GpuWrapper::create_data(const RenderProcedural& render_procedural,
                                                                   uint32_t vertex_count)
    {
        auto size = render_procedural.vertex_layout().get_stride() * vertex_count;
        auto vertex_buffer = BufferWrapper::Builder{}.set_flag(SDL_GPU_BUFFERUSAGE_VERTEX).set_size(size).build(*this);
        if (!vertex_buffer)
        {
            return std::unexpected(vertex_buffer.error());
        }
        auto index_buffer =
            BufferWrapper::Builder{}.set_flag(SDL_GPU_BUFFERUSAGE_INDEX).set_size(6 * sizeof(int)).build(*this);
        if (!index_buffer)
        {
            return std::unexpected(index_buffer.error());
        }
        return std::make_shared<RenderDataImpl>(std::move(vertex_buffer.value()), std::move(index_buffer.value()),
                                                render_procedural.vertex_layout(), vertex_count);
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
