//
// Created by sophomore on 11/11/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
export module sdl_wrapper:pipeline;
import :decl;
export namespace sopho
{
    class PipelineWrapper
    {
        SDL_GPUGraphicsPipeline* m_graphics_pipeline{};
        std::shared_ptr<GpuWrapper> m_device{};
        SDL_GPUGraphicsPipelineCreateInfo m_pipeline_info{};
        bool modified = false;

        PipelineWrapper(std::shared_ptr<GpuWrapper> p_device);
    public:
        ~PipelineWrapper();

        auto data() { return m_graphics_pipeline; }

        auto submit();
        friend GpuWrapper;
    };
} // namespace sopho
