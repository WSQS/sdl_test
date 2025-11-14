//
// Created by wsqsy on 11/14/2025.
//
module;
#include <SDL3/SDL_gpu.h>
module sdl_wrapper;
import :gpu;
import :pipeline;
namespace sopho
{
    BufferWrapper GpuWrapper::create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size)
    {
        SDL_GPUBufferCreateInfo create_info{flag, size};
        auto buffer = SDL_CreateGPUBuffer(m_device, &create_info);
        BufferWrapper result(shared_from_this(), buffer);
        return result;
    }
    PipelineWrapper GpuWrapper::create_pipeline() { return PipelineWrapper{shared_from_this()}; }
} // namespace sopho
