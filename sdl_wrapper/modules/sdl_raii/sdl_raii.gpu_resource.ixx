// sdl_raii.gpu_resource.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:gpu_resource;
namespace sopho
{
    template <typename T>
    struct GpuResourceTraits;

    // Due to Windows msvc problem, we need to put all template in one module partition.

    template <>
    struct GpuResourceTraits<SDL_GPUBuffer>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUBuffer* buffer) noexcept { SDL_ReleaseGPUBuffer(device, buffer); }
    };
    template <>
    struct GpuResourceTraits<SDL_GPUGraphicsPipeline>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUGraphicsPipeline* raw) noexcept
        {
            SDL_ReleaseGPUGraphicsPipeline(device, raw);
        }
    };
    template <>
    struct GpuResourceTraits<SDL_GPUSampler>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUSampler* raw) noexcept { SDL_ReleaseGPUSampler(device, raw); }
    };
    template <>
    struct GpuResourceTraits<SDL_GPUShader>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUShader* raw) noexcept { SDL_ReleaseGPUShader(device, raw); }
    };
    template <>
    struct GpuResourceTraits<SDL_GPUTexture>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUTexture* raw) noexcept { SDL_ReleaseGPUTexture(device, raw); }
    };
    template <>
    struct GpuResourceTraits<SDL_GPUTransferBuffer>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUTransferBuffer* buffer) noexcept
        {
            SDL_ReleaseGPUTransferBuffer(device, buffer);
        }
    };


} // namespace sopho
