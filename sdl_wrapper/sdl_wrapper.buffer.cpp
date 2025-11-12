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
    /**
     * @brief Construct a BufferWrapper that allocates and manages a GPU vertex buffer.
     *
     * Initializes the wrapper with the given GPU context and creates a GPU vertex buffer
     * using the provided creation parameters, storing the associated GPU wrapper.
     *
     * @param p_gpu Shared pointer to the associated GpuWrapper; must be non-null and remain valid
     *              for the lifetime of this BufferWrapper.
     * @param p_create_info Parameters that describe the GPU buffer to create.
     */
    BufferWrapper::BufferWrapper(std::shared_ptr<GpuWrapper> p_gpu, const SDL_GPUBufferCreateInfo& p_create_info) :
        m_gpu(p_gpu), m_vertex_buffer(SDL_CreateGPUBuffer(m_gpu->data(), &p_create_info))
    {
    }
    /**
     * @brief Releases GPU resources owned by this BufferWrapper.
     *
     * Releases the GPU vertex buffer and, if present, the transfer (staging) buffer,
     * then clears the corresponding handles and resets the transfer buffer size.
     *
     * @note If the associated GPU has already been destroyed, releasing these resources
     *       may have no effect or may be too late to perform a proper cleanup.
     */
    BufferWrapper::~BufferWrapper()
    {
        // TODO: It's too late to release gpu buffer, gpu was released
        SDL_ReleaseGPUBuffer(m_gpu->data(), m_vertex_buffer);
        m_vertex_buffer = nullptr;
        if (m_transfer_buffer)
        {
            SDL_ReleaseGPUTransferBuffer(m_gpu->data(), m_transfer_buffer);
            m_transfer_buffer = nullptr;
            m_transfer_buffer_size = 0;
        }
    }

    /**
     * @brief Uploads a block of data into the wrapped GPU vertex buffer at the specified offset.
     *
     * If the internal transfer (staging) buffer is smaller than `p_size`, it will be reallocated
     * to accommodate the upload. The function copies `p_size` bytes from `p_data` into the transfer
     * buffer and enqueues a GPU copy pass that transfers those bytes into the vertex buffer at
     * `p_offset`.
     *
     * @param p_data Pointer to the source data to upload.
     * @param p_size Size in bytes of the data to upload.
     * @param p_offset Byte offset within the vertex buffer where the data will be written.
     */
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