// sdl_raii.shader.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:shader;
import :gpu_resource;
namespace sopho
{
    template<>
    struct GpuResourceTraits<SDL_GPUShader>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUShader* shader) noexcept
        {
            SDL_ReleaseGPUShader(device, shader);
        }
    };
    using GpuShaderRaii = GpuResourceRaii<SDL_GPUShader>;
} // namespace sopho
