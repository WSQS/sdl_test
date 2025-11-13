//
// Created by sophomore on 11/11/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "shaderc/shaderc.hpp"
export module sdl_wrapper:pipeline;
import :decl;
export namespace sopho
{
    class PipelineWrapper
    {
        SDL_GPUGraphicsPipeline* m_graphics_pipeline{};
        std::shared_ptr<GpuWrapper> m_device{};

        SDL_GPUShader* m_vertex_shader{};
        SDL_GPUShader* m_fragment_shader{};
        std::vector<SDL_GPUVertexBufferDescription> m_vertex_buffer_description{};
        std::vector<SDL_GPUVertexAttribute> m_vertex_attribute{};
        std::vector<SDL_GPUColorTargetDescription> m_color_target_description{};
        SDL_GPUGraphicsPipelineCreateInfo m_pipeline_info{};

        shaderc::Compiler compiler{};
        shaderc::CompileOptions options{};

        bool m_modified = false;

    public:
        PipelineWrapper(std::shared_ptr<GpuWrapper> p_device);
        PipelineWrapper(const PipelineWrapper&)=default;
        PipelineWrapper(PipelineWrapper&&)=default;
        ~PipelineWrapper();

        auto data() { return m_graphics_pipeline; }

        void submit();

        void set_vertex_shader(const std::string& p_source);
        void set_fragment_shader(const std::string& p_source);
        friend GpuWrapper;
    };
} // namespace sopho
