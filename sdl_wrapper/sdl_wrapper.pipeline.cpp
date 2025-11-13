//
// Created by sophomore on 11/13/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
module sdl_wrapper;
import :pipeline;

namespace sopho
{
    /**
     * @brief Initializes the PipelineWrapper with the given GPU device wrapper.
     *
     * @param p_device Shared pointer to a GpuWrapper representing the target GPU device; the wrapper retains this
     * reference for its lifetime.
     */
    PipelineWrapper::PipelineWrapper(std::shared_ptr<GpuWrapper> p_device) : m_device(p_device) {}
    /**
     * @brief Releases any GPU graphics pipeline owned by this wrapper.
     *
     * If a graphics pipeline is currently held, it is released using the associated
     * device and the stored pipeline handle is cleared so the wrapper no longer
     * references the pipeline.
     */
    PipelineWrapper::~PipelineWrapper()
    {
        if (m_graphics_pipeline)
        {
            SDL_ReleaseGPUGraphicsPipeline(m_device->data(), m_graphics_pipeline);
            m_graphics_pipeline = nullptr;
        }
    }
    /**
     * @brief Rebuilds the GPU graphics pipeline when the wrapper is marked modified.
     *
     * If the wrapper's modified flag is set, this clears the flag, attempts to create a new
     * graphics pipeline from the stored pipeline info and device, and on success replaces the
     * current pipeline (releasing the previous pipeline first). If creation fails, an error
     * is logged.
     */
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
