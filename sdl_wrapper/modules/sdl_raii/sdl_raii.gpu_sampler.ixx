// sdl_raii.gpu_sampler.ixx
// Created by sophomore on 11/28/25.
//
module;
#include "SDL3/SDL_gpu.h"
export module sdl_raii:gpu_sampler;
import :gpu_resource;
namespace sopho
{
    template <>
    struct GpuResourceTraits<SDL_GPUSampler>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUSampler* raw) noexcept { SDL_ReleaseGPUSampler(device, raw); }
    };
} // namespace sopho
