// sdl_raii.pure_traits.ixx
// Created by sophomore on 11/29/25.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:pure_traits;
namespace sopho
{
    template <typename T>
    struct PureTraits;

    // Due to Windows msvc problem, we need to put all template in one module partition.

    template <>
    struct PureTraits<SDL_GPUCommandBuffer>
    {
        static void release(SDL_GPUCommandBuffer* raw) noexcept { SDL_SubmitGPUCommandBuffer(raw); }
    };
    template <>
    struct PureTraits<SDL_GPUDevice>
    {
        static void release(SDL_GPUDevice* raw) noexcept { SDL_DestroyGPUDevice(raw); }
    };
} // namespace sopho
