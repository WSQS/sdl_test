// sdl_raii.gpu_transfer_buffer.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:gpu_transfer_buffer;
import :gpu_resource;
namespace sopho
{
    template <>
    struct GpuResourceTraits<SDL_GPUTransferBuffer>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUTransferBuffer* raw) noexcept
        {
            SDL_ReleaseGPUTransferBuffer(device, raw);
        }
    };
} // namespace sopho
