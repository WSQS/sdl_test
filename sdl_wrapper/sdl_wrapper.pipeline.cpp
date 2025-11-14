//
// Created by sophomore on 11/13/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "shaderc/shaderc.hpp"
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
    PipelineWrapper::PipelineWrapper(std::shared_ptr<GpuWrapper> p_device) : m_device(p_device)
    {
        options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);

        m_vertex_buffer_description.emplace_back(0, 28, SDL_GPU_VERTEXINPUTRATE_VERTEX, 0);
        m_pipeline_info.vertex_input_state.vertex_buffer_descriptions = m_vertex_buffer_description.data();
        m_pipeline_info.vertex_input_state.num_vertex_buffers = m_vertex_buffer_description.size();

        m_vertex_attribute.emplace_back(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0);
        m_vertex_attribute.emplace_back(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, sizeof(float) * 3);
        m_pipeline_info.vertex_input_state.vertex_attributes = m_vertex_attribute.data();
        m_pipeline_info.vertex_input_state.num_vertex_attributes = m_vertex_attribute.size();

        m_pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;

        m_color_target_description.emplace_back(m_device->get_texture_format(),
                                                SDL_GPUColorTargetBlendState{SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                                                                             SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                                                                             SDL_GPU_BLENDOP_ADD,
                                                                             SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                                                                             SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                                                                             SDL_GPU_BLENDOP_ADD,
                                                                             {},
                                                                             true});

        m_pipeline_info.target_info.color_target_descriptions = m_color_target_description.data();
        m_pipeline_info.target_info.num_color_targets = m_color_target_description.size();

        m_modified = true;
    }

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
        m_device->release_shader(m_vertex_shader);
        m_device->release_shader(m_fragment_shader);
    }
    /**
     * @brief Rebuilds the GPU graphics pipeline when the wrapper is marked modified.
     *
     * If the wrapper's modified flag is set, this clears the flag, attempts to create a new
     * graphics pipeline from the stored pipeline info and device, and on success replaces the
     * current pipeline (releasing the previous pipeline first). If creation fails, an error
     * is logged.
     */
    void PipelineWrapper::submit()
    {
        if (m_modified)
        {
            m_modified = false;
            m_pipeline_info.vertex_input_state.vertex_buffer_descriptions = m_vertex_buffer_description.data();
            m_pipeline_info.vertex_input_state.vertex_attributes = m_vertex_attribute.data();
            m_pipeline_info.target_info.color_target_descriptions = m_color_target_description.data();
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

    void PipelineWrapper::set_vertex_shader(const std::string& p_source)
    {
        auto result = compiler.CompileGlslToSpv(p_source, shaderc_glsl_vertex_shader, "vertex.glsl", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[shaderc] compile error in vertex.glsl: %s",
                         result.GetErrorMessage().data());
        }
        else
        {
            m_device->release_shader(m_vertex_shader);
            auto code_size = static_cast<size_t>(result.cend() - result.cbegin()) * sizeof(uint32_t);
            auto ptr = reinterpret_cast<const uint8_t*>(result.cbegin());
            std::vector<uint8_t> code{ptr, ptr + code_size};
            m_vertex_shader = m_device->create_shader(code, SDL_GPU_SHADERSTAGE_VERTEX);
            m_pipeline_info.vertex_shader = m_vertex_shader;
            m_modified = true;
        }
    }
    void PipelineWrapper::set_fragment_shader(const std::string& p_source)
    {
        auto result = compiler.CompileGlslToSpv(p_source, shaderc_glsl_fragment_shader, "fragment.glsl", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[shaderc] compile error fragment.glsl: %s",
                         result.GetErrorMessage().data());
        }
        else
        {
            m_device->release_shader(m_fragment_shader);
            auto code_size = static_cast<size_t>(result.cend() - result.cbegin()) * sizeof(uint32_t);
            auto ptr = reinterpret_cast<const uint8_t*>(result.cbegin());
            std::vector<uint8_t> code{ptr, ptr + code_size};
            m_fragment_shader = m_device->create_shader(code, SDL_GPU_SHADERSTAGE_FRAGMENT);
            m_pipeline_info.fragment_shader = m_fragment_shader;
            m_modified = true;
        }
    }
} // namespace sopho
