// sdl_raii.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii;

export import :bound_traits;
export import :bound_raii;
export import :pure_traits;
export import :pure_raii;

export namespace sopho
{
    using GpuBufferRaii = BoundRaii<SDL_GPUBuffer>;
    using GPUGraphicsPipelineRaii = BoundRaii<SDL_GPUGraphicsPipeline>;
    using GPUSamplerRaii = BoundRaii<SDL_GPUSampler>;
    using GpuShaderRaii = BoundRaii<SDL_GPUShader>;
    using GpuTextureRaii = BoundRaii<SDL_GPUTexture>;
    using GpuTransferBufferRaii = BoundRaii<SDL_GPUTransferBuffer>;
    using ClaimWindowRaii = BoundRaii<SDL_Window>;
    using GpuCommandBufferRaii = PureRaii<SDL_GPUCommandBuffer>;
    using GpuDeviceRaii = PureRaii<SDL_GPUDevice>;
    using WindowRaii = PureRaii<SDL_Window>;
} // namespace sopho
