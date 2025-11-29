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

        for (int i = 0;i<std::min<int>(size,vertex_data.size());++i)
        {
            vertex_buffer->cpu_buffer()[i] = vertex_data[i];
        }

        int* index_ptr = reinterpret_cast<int*>(index_buffer.value().cpu_buffer());
        index_ptr[0] = 0;
        index_ptr[1] = 1;
        index_ptr[2] = 2;
        index_ptr[3] = 1;
        index_ptr[4] = 2;
        index_ptr[5] = 3;
        index_ptr[6] = 0;
        index_ptr[7] = 1;
        index_ptr[8] = 4;
        index_ptr[9] = 1;
        index_ptr[10] = 4;
        index_ptr[11] = 5;
        index_ptr[12] = 0;
        index_ptr[13] = 2;
        index_ptr[14] = 4;
        index_ptr[15] = 2;
        index_ptr[16] = 4;
        index_ptr[17] = 6;
        index_ptr[18] = 2;
        index_ptr[19] = 3;
        index_ptr[20] = 6;
        index_ptr[21] = 3;
        index_ptr[22] = 6;
        index_ptr[23] = 7;
        index_ptr[24] = 1;
        index_ptr[25] = 3;
        index_ptr[26] = 5;
        index_ptr[27] = 3;
        index_ptr[28] = 5;
        index_ptr[29] = 7;
        index_ptr[30] = 4;
        index_ptr[31] = 5;
        index_ptr[32] = 6;
        index_ptr[33] = 5;
        index_ptr[34] = 6;
        index_ptr[35] = 7;
        return std::make_shared<RenderDataImpl>(std::move(vertex_buffer.value()), std::move(index_buffer.value()),
                                                layout, vertex_count, index_count);
    }
} // namespace sopho
