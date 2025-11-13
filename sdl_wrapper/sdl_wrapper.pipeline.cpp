//
// Created by sophomore on 11/13/25.
//
module;
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include <memory>
module sdl_wrapper;
import :pipeline;

namespace sopho
{
    PipelineWrapper::PipelineWrapper(std::shared_ptr<GpuWrapper> p_device) : m_device(p_device) {}
    PipelineWrapper::~PipelineWrapper()
    {
        if (m_graphics_pipeline)
        {
            SDL_ReleaseGPUGraphicsPipeline(m_device->data(), m_graphics_pipeline);
            m_graphics_pipeline = nullptr;
        }
    }
    auto PipelineWrapper::submit()
    {
        if (modified)
        {
            modified = false;
            auto new_graphics_pipeline = SDL_CreateGPUGraphicsPipeline(m_device->data(), &m_pipeline_info);
            if (new_graphics_pipeline)
            {
                if (m_graphics_pipeline)
                {
                    SDL_ReleaseGPUGraphicsPipeline(m_device->data(), m_graphics_pipeline);
                }
                m_graphics_pipeline = new_graphics_pipeline;
            }
            else
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s get error:%s", __FUNCTION__, SDL_GetError());
            }
        }
    }
} // namespace sopho
