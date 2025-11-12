//
// Created by sophomore on 11/11/25.
//
module;
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
export module sdl_wrapper:pipeline;

namespace sopho
{
    export class PipelineWrapper
    {
        SDL_GPUGraphicsPipeline* m_graphics_pipeline{};
        SDL_GPUDevice* m_device{};
        SDL_GPUGraphicsPipelineCreateInfo m_pipeline_info{};
        bool modified = false;

    public:
        PipelineWrapper() = default;
        ~PipelineWrapper()
        {
            if (m_graphics_pipeline)
            {
                SDL_ReleaseGPUGraphicsPipeline(m_device, m_graphics_pipeline);
                m_graphics_pipeline = nullptr;
            }
        }

        auto data() { return m_graphics_pipeline; }

        auto submit()
        {
            if (modified)
            {
                modified = false;
                auto new_graphics_pipeline = SDL_CreateGPUGraphicsPipeline(m_device, &m_pipeline_info);
                if (new_graphics_pipeline) {
                    if (m_graphics_pipeline) {
                        SDL_ReleaseGPUGraphicsPipeline(m_device, m_graphics_pipeline);
                    }
                    m_graphics_pipeline = new_graphics_pipeline;
                } else {
                    SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s get error:%s",__FUNCTION__, SDL_GetError());
                }
            }
        }
    };
} // namespace sopho
