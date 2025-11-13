//
// Created by sophomore on 11/12/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
module sdl_wrapper;
import :buffer;
import :gpu;

namespace sopho
{

    BufferWrapper::~BufferWrapper()
    {
        SDL_ReleaseGPUBuffer(m_gpu->data(), m_vertex_buffer);
        m_vertex_buffer = nullptr;
        if (m_transfer_buffer)
        {
            SDL_ReleaseGPUTransferBuffer(m_gpu->data(), m_transfer_buffer);
            m_transfer_buffer = nullptr;
            m_transfer_buffer_size = 0;
        }
    }

    void BufferWrapper::upload(void* p_data, uint32_t p_size, uint32_t p_offset)
    {
        if (p_size > m_transfer_buffer_size)
        {
            if (m_transfer_buffer != nullptr)
            {
                SDL_ReleaseGPUTransferBuffer(m_gpu->data(), m_transfer_buffer);
            }
            SDL_GPUTransferBufferCreateInfo transfer_info{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, p_size, 0};
            m_transfer_buffer = SDL_CreateGPUTransferBuffer(m_gpu->data(), &transfer_info);
            m_transfer_buffer_size = transfer_info.size;
        }

        auto data = SDL_MapGPUTransferBuffer(m_gpu->data(), m_transfer_buffer, false);
        SDL_memcpy(data, p_data, p_size);
        SDL_UnmapGPUTransferBuffer(m_gpu->data(), m_transfer_buffer);

        // TODO: Delay submit command in collect
        auto command_buffer = SDL_AcquireGPUCommandBuffer(m_gpu->data());
        auto copy_pass = SDL_BeginGPUCopyPass(command_buffer);

        SDL_GPUTransferBufferLocation location{};
        location.transfer_buffer = m_transfer_buffer;
        location.offset = 0;

        SDL_GPUBufferRegion region{};
        region.buffer = m_vertex_buffer;
        region.size = p_size;
        region.offset = p_offset;

        SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);

        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(command_buffer);
    }
} // namespace sopho
