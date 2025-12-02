// sdl_raii.bound_traits.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:bound_traits;
namespace sopho
{
    template <typename T>
    struct BoundTraits;

    // Due to Windows msvc problem, we need to put all template in one module partition.

    template <>
    struct BoundTraits<SDL_GPUBuffer>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUBuffer* buffer) noexcept { SDL_ReleaseGPUBuffer(device, buffer); }
    };
    template <>
    struct BoundTraits<SDL_GPUGraphicsPipeline>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUGraphicsPipeline* raw) noexcept
        {
            SDL_ReleaseGPUGraphicsPipeline(device, raw);
        }
    };
    template <>
    struct BoundTraits<SDL_GPUSampler>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUSampler* raw) noexcept { SDL_ReleaseGPUSampler(device, raw); }
    };
    template <>
    struct BoundTraits<SDL_GPUShader>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUShader* raw) noexcept { SDL_ReleaseGPUShader(device, raw); }
    };
    template <>
    struct BoundTraits<SDL_GPUTexture>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUTexture* raw) noexcept { SDL_ReleaseGPUTexture(device, raw); }
    };
    template <>
    struct BoundTraits<SDL_GPUTransferBuffer>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUTransferBuffer* buffer) noexcept
        {
            SDL_ReleaseGPUTransferBuffer(device, buffer);
        }
    };
    template <>
    struct BoundTraits<SDL_Window>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_Window* raw) noexcept { SDL_ReleaseWindowFromGPUDevice(device, raw); }
    };


} // namespace sopho
