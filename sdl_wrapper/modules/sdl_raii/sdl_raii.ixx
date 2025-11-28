// sdl_raii.ixx
// Created by wsqsy on 11/28/2025.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii;

export import :gpu_device;
export import :window;
export import :claim_window;
export import :gpu_transfer_buffer;
export import :gpu_buffer;
export import :gpu_shader;
export import :gpu_graphics_pipeline;
export import :gpu_texture;
export import :gpu_sampler;
export import :gpu_resource;

export namespace sopho
{
    using GpuTextureRaii = GpuResourceRaii<SDL_GPUTexture>;
    using GpuShaderRaii = GpuResourceRaii<SDL_GPUShader>;
    using GpuTransferBufferRaii = GpuResourceRaii<SDL_GPUTransferBuffer>;
    using GPUSamplerRaii = GpuResourceRaii<SDL_GPUSampler>;
    using GPUGraphicsPipelineRaii = GpuResourceRaii<SDL_GPUGraphicsPipeline>;
    using GpuBufferRaii = GpuResourceRaii<SDL_GPUBuffer>;
} // namespace sopho
