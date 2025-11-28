// sdl_raii.gpu_graphics_pipeline.ixx
// Created by sophomore on 11/28/25.
//
module;
#include <SDL3/SDL_gpu.h>
export module sdl_raii:gpu_graphics_pipeline;
import :gpu_resource;
namespace sopho
{
    template <>
    struct GpuResourceTraits<SDL_GPUGraphicsPipeline>
    {
        using Device = SDL_GPUDevice;

        static void release(Device* device, SDL_GPUGraphicsPipeline* raw) noexcept
        {
            SDL_ReleaseGPUGraphicsPipeline(device, raw);
        }
    };
    export using GPUGraphicsPipelineRaii = GpuResourceRaii<SDL_GPUGraphicsPipeline>;
} // namespace sopho
