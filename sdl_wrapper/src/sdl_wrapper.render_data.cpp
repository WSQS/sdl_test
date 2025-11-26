// sdl_wrapper.render_data.cpp
// Created by sophomore on 11/26/25.
//
module;
#include <expected>
#include <memory>
#include "SDL3/SDL_gpu.h"
module sdl_wrapper;
import :buffer;
import :render_data_impl;
import :render_data;
import :vertex_layout;
namespace sopho
{

    checkable<std::shared_ptr<RenderData>> RenderData::Builder::build(GpuWrapper& gpu)
    {
        auto size = layout.get_stride() * vertex_count;
        auto vertex_buffer = BufferWrapper::Builder{}.set_flag(SDL_GPU_BUFFERUSAGE_VERTEX).set_size(size).build(gpu);
        if (!vertex_buffer)
        {
            return std::unexpected(vertex_buffer.error());
        }
        auto index_buffer =
            BufferWrapper::Builder{}.set_flag(SDL_GPU_BUFFERUSAGE_INDEX).set_size(index_count * sizeof(int)).build(gpu);
        if (!index_buffer)
        {
            return std::unexpected(index_buffer.error());
        }
        return std::make_shared<RenderDataImpl>(std::move(vertex_buffer.value()), std::move(index_buffer.value()),
                                                layout, vertex_count, index_count);
    }
} // namespace sopho
