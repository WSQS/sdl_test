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
        float* vertex_ptr = reinterpret_cast<float*>(vertex_buffer.value().cpu_buffer());
        vertex_ptr[0] = 0.5;
        vertex_ptr[1] = 0.5;
        vertex_ptr[2] = 0.5;
        vertex_ptr[5] = -0.5;
        vertex_ptr[6] = 0.5;
        vertex_ptr[7] = 0.5;
        vertex_ptr[10] = 0.5;
        vertex_ptr[11] = -0.5;
        vertex_ptr[12] = 0.5;
        vertex_ptr[15] = -0.5;
        vertex_ptr[16] = -0.5;
        vertex_ptr[17] = 0.5;
        vertex_ptr[20] = 0.5;
        vertex_ptr[21] = 0.5;
        vertex_ptr[22] = -0.5;
        vertex_ptr[25] = -0.5;
        vertex_ptr[26] = 0.5;
        vertex_ptr[27] = -0.5;
        vertex_ptr[30] = 0.5;
        vertex_ptr[31] = -0.5;
        vertex_ptr[32] = -0.5;
        vertex_ptr[35] = -0.5;
        vertex_ptr[36] = -0.5;
        vertex_ptr[37] = -0.5;

        // v0 uv
        vertex_ptr[3]  = 0.0f;
        vertex_ptr[4]  = 0.0f;

        // v1 uv
        vertex_ptr[8]  = 1.0f;
        vertex_ptr[9]  = 0.0f;

        // v2 uv
        vertex_ptr[13] = 0.0f;
        vertex_ptr[14] = 1.0f;

        // v3 uv
        vertex_ptr[18] = 1.0f;
        vertex_ptr[19] = 1.0f;

        // v4 uv
        vertex_ptr[23] = 1.0f;
        vertex_ptr[24] = 1.0f;

        // v5 uv
        vertex_ptr[28] = 0.0f;
        vertex_ptr[29] = 1.0f;

        // v6 uv
        vertex_ptr[33] = 1.0f;
        vertex_ptr[34] = 0.0f;

        // v7 uv
        vertex_ptr[38] = 0.0f;
        vertex_ptr[39] = 0.0f;

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
