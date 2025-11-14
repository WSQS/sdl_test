//
// Created by sophomore on 11/8/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
export module sdl_wrapper:buffer;
import :decl;

export namespace sopho
{
    class BufferWrapper
    {
        std::shared_ptr<GpuWrapper> m_gpu{};
        SDL_GPUBuffer* m_vertex_buffer{};
        SDL_GPUTransferBuffer* m_transfer_buffer{};
        uint32_t m_transfer_buffer_size{};

        BufferWrapper(std::shared_ptr<::sopho::GpuWrapper> p_gpu, SDL_GPUBuffer* p_buffer) :
            m_gpu(p_gpu), m_vertex_buffer(p_buffer)
        {
        }

    public:
        void upload(void* p_data, uint32_t p_size, uint32_t p_offset);

        auto data() { return m_vertex_buffer; }

        ~BufferWrapper();
        friend GpuWrapper;
    };
} // namespace sopho
