// sdl_raii.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii;

export import :gpu_device;
export import :window;
export import :claim_window;
export import :gpu_resource;
export import :gpu_resource_raii;

export namespace sopho
{
    using GpuBufferRaii = GpuResourceRaii<SDL_GPUBuffer>;
    using GPUGraphicsPipelineRaii = GpuResourceRaii<SDL_GPUGraphicsPipeline>;
    using GPUSamplerRaii = GpuResourceRaii<SDL_GPUSampler>;
    using GpuShaderRaii = GpuResourceRaii<SDL_GPUShader>;
    using GpuTextureRaii = GpuResourceRaii<SDL_GPUTexture>;
    using GpuTransferBufferRaii = GpuResourceRaii<SDL_GPUTransferBuffer>;
} // namespace sopho
