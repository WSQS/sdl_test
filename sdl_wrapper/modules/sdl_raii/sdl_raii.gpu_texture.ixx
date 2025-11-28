// sdl_raii.gpu_texture.ixx
// Created by sophomore on 11/28/25.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:gpu_texture;
import :gpu_resource;
namespace sopho
{
    template <>
    struct GpuResourceTraits<SDL_GPUTexture>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUTexture* raw) noexcept { SDL_ReleaseGPUTexture(device, raw); }
    };
    export using GpuTextureRaii = GpuResourceRaii<SDL_GPUTexture>;
} // namespace sopho
