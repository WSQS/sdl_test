// sdl_raii.gpu_buffer.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:gpu_buffer;
import :gpu_resource;
namespace sopho
{
    template <>
    struct GpuResourceTraits<SDL_GPUBuffer>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUBuffer* buffer) noexcept { SDL_ReleaseGPUBuffer(device, buffer); }
    };
    export using GpuBufferRaii = GpuResourceRaii<SDL_GPUBuffer>;
} // namespace sopho
