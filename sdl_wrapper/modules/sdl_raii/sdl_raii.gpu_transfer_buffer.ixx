// sdl_raii.transfer_buffer.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:transfer_buffer;
import :gpu_resource;
namespace sopho
{
    template <>
    struct GpuResourceTraits<SDL_GPUTransferBuffer>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUTransferBuffer* buffer) noexcept
        {
            SDL_ReleaseGPUTransferBuffer(device, buffer);
        }
    };
    export using GpuTransferBufferRaii = GpuResourceRaii<SDL_GPUTransferBuffer>;
} // namespace sopho
