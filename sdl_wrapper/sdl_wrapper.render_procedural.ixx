// sdl_wrapper.pipeline.ixx
// Created by sophomore on 11/11/25.
//
module;
#include <expected>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "SDL3/SDL_gpu.h"
#include "shaderc/shaderc.hpp"
export module sdl_wrapper:render_procedural;
import :decl; // GpuError, forward declarations, etc.
import :vertex_layout;
export namespace sopho
{
    class RenderProcedural
    {
        std::shared_ptr<GpuWrapper> m_gpu{};

        SDL_GPUGraphicsPipeline* m_graphics_pipeline{};
        SDL_GPUShader* m_vertex_shader{};
        SDL_GPUShader* m_fragment_shader{};

        std::vector<SDL_GPUVertexBufferDescription> m_vertex_buffer_descriptions{};
        std::vector<SDL_GPUColorTargetDescription> m_color_target_descriptions{};
        SDL_GPUGraphicsPipelineCreateInfo m_pipeline_info{};

        shaderc::Compiler m_compiler{};
        shaderc::CompileOptions m_options{};

        VertexLayout m_vertex_layout{};
        bool m_modified{false};

        // Internal constructor: assumes texture format is already known and valid.
        RenderProcedural(std::shared_ptr<GpuWrapper> gpu, SDL_GPUTextureFormat swapchain_format) noexcept;

    public:
        RenderProcedural(const RenderProcedural&) = delete;
        RenderProcedural& operator=(const RenderProcedural&) = delete;
        RenderProcedural(RenderProcedural&&) noexcept = default;
        RenderProcedural& operator=(RenderProcedural&&) = delete;
        ~RenderProcedural() noexcept;

        /// Returns the underlying SDL_GPUGraphicsPipeline*.
        [[nodiscard]] SDL_GPUGraphicsPipeline* data() const noexcept { return m_graphics_pipeline; }

        /// Rebuilds the graphics pipeline if any state has been modified.
        ///
        /// On success, this replaces the previous pipeline (if any) with a newly created one.
        /// On failure, an error is propagated to the caller.
        [[nodiscard]] std::expected<std::monostate, GpuError> submit();

        /// Compiles and sets the vertex shader from GLSL source.
        ///
        /// On success, this replaces the previous vertex shader (if any), updates the
        /// pipeline create info, and marks the pipeline as modified.
        /// On failure, a compile or creation error is returned.
        [[nodiscard]] std::expected<std::monostate, GpuError> set_vertex_shader(const std::string& source);

        /// Compiles and sets the fragment shader from GLSL source.
        ///
        /// On success, this replaces the previous fragment shader (if any), updates the
        /// pipeline create info, and marks the pipeline as modified.
        /// On failure, a compile or creation error is returned.
        [[nodiscard]] std::expected<std::monostate, GpuError> set_fragment_shader(const std::string& source);

        auto set_vertex_reflection(const VertexReflection& vertex_attributes)
        {
            m_vertex_layout.set_vertex_reflection(vertex_attributes);

            // Setup vertex buffer description.
            m_vertex_buffer_descriptions.clear();
            SDL_GPUVertexBufferDescription vb_desc{};
            vb_desc.slot = 0;
            vb_desc.pitch = m_vertex_layout.get_stride();
            vb_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
            vb_desc.instance_step_rate = 0;
            m_vertex_buffer_descriptions.push_back(vb_desc);

            m_pipeline_info.vertex_input_state.vertex_attributes = m_vertex_layout.get_vertex_attributes().data();
            m_pipeline_info.vertex_input_state.num_vertex_attributes =
                static_cast<std::uint32_t>(m_vertex_layout.get_vertex_attributes().size());

            m_modified = true;
        }

        auto& vertex_layout() const { return m_vertex_layout; }

        friend class GpuWrapper;
    };
} // namespace sopho
