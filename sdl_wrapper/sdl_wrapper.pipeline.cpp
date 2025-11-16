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
     * @brief Constructs a PipelineWrapper and configures default pipeline state for the provided GPU device.
     *
     * Stores the provided GPU device wrapper for the wrapper's lifetime, configures the shaderc target environment,
     * and initializes default vertex input state, primitive type, and color target description used when creating
     * graphics pipelines.
     *
     * @param p_device Shared pointer to the GpuWrapper used to create and release shaders and graphics pipelines.
     */
    PipelineWrapper::PipelineWrapper(std::shared_ptr<::sopho::GpuWrapper> p_device) : m_device(p_device)
    {
        options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);

        m_vertex_buffer_description.emplace_back(0, 28, SDL_GPU_VERTEXINPUTRATE_VERTEX, 0);
        m_pipeline_info.vertex_input_state.vertex_buffer_descriptions = m_vertex_buffer_description.data();
        m_pipeline_info.vertex_input_state.num_vertex_buffers = m_vertex_buffer_description.size();

        m_vertex_attribute.emplace_back(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0);
        m_vertex_attribute.emplace_back(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, sizeof(float) * 3);
        m_pipeline_info.vertex_input_state.vertex_attributes = m_vertex_attribute.data();
        m_pipeline_info.vertex_input_state.num_vertex_attributes = m_vertex_attribute.size();

        m_pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

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

    /**
     * @brief Compile and install a new vertex shader from GLSL source.
     *
     * Compiles the provided GLSL vertex shader source to SPIR‑V, replaces any previously installed
     * vertex shader on the device with the newly created shader, updates the pipeline's vertex
     * shader reference, and marks the pipeline wrapper as modified so the pipeline will be rebuilt.
     *
     * @param p_source GLSL source code for the vertex shader.
     *
     * On compilation failure, a compilation error is logged and the previously installed shader is left unchanged.
     */
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
            m_vertex_shader = m_device->create_shader(code, SDL_GPU_SHADERSTAGE_VERTEX, 1);
            m_pipeline_info.vertex_shader = m_vertex_shader;
            m_modified = true;
        }
    }
    /**
     * @brief Compile and install a fragment shader from GLSL source.
     *
     * Compiles the provided GLSL fragment shader source to SPIR‑V; on compilation failure logs the error.
     * On success, releases the previously installed fragment shader (if any), uploads the new SPIR‑V bytecode
     * to the device as a fragment-stage shader, updates the internal pipeline description to reference it,
     * and marks the wrapper as modified so the graphics pipeline will be rebuilt.
     *
     * @param p_source GLSL source code for the fragment shader.
     */
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
            m_fragment_shader = m_device->create_shader(code, SDL_GPU_SHADERSTAGE_FRAGMENT, 0);
            m_pipeline_info.fragment_shader = m_fragment_shader;
            m_modified = true;
        }
    }
} // namespace sopho
