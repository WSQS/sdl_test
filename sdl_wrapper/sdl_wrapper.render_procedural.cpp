// sdl_wrapper.pipeline.cpp
// Created by sophomore on 11/13/25.
//
module;
#include <expected>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "shaderc/shaderc.hpp"
module sdl_wrapper;
import :render_procedural;
import :gpu;
import glsl_reflector;
namespace sopho
{
    /**
     * @brief Convert a shaderc SPIR-V compilation result into a contiguous byte buffer.
     *
     * Converts the iterable sequence of 32-bit words produced by shaderc into a
     * std::vector<std::uint8_t> containing the raw SPIR-V bytes in order.
     *
     * @param result The shaderc SPIR-V compilation result (iterable of uint32_t words).
     * @return std::vector<std::uint8_t> Raw SPIR-V byte sequence suitable for creating GPU shader modules.
     */
    static std::vector<std::uint8_t> spv_result_to_bytes(const shaderc::SpvCompilationResult& result)
    {
        // shaderc::SpvCompilationResult is an iterable sequence of uint32_t words.
        auto begin = result.cbegin();
        auto end = result.cend();
        const std::size_t word_count = static_cast<std::size_t>(end - begin);

        std::vector<std::uint8_t> bytes;
        bytes.reserve(word_count * sizeof(std::uint32_t));

        // Safely read each word and append its bytes.
        for (auto it = begin; it != end; ++it)
        {
            std::uint32_t word = *it;
            const auto* ptr = reinterpret_cast<const std::uint8_t*>(&word);
            bytes.insert(bytes.end(), ptr, ptr + sizeof(std::uint32_t));
        }

        return bytes;
    }

    /**
     * @brief Construct a RenderProcedural configured for the given GPU and swapchain format.
     *
     * Initializes default GPU pipeline state: vertex layout (positions + colors), shaderc options
     * targeting Vulkan/SPIR-V, a vertex buffer description, an alpha blend color target using the
     * provided swapchain format, and a default graphics pipeline create-info with no shaders.
     * Marks the pipeline as modified so it will be (re)created on the next submit.
     *
     * @param swapchain_format Pixel format to use for the pipeline's color target.
     */
    RenderProcedural::RenderProcedural(std::shared_ptr<GpuWrapper> gpu, SDL_GPUTextureFormat swapchain_format) noexcept
        : m_gpu(std::move(gpu))
    {
        // Configure shaderc to target Vulkan/SPIR-V.
        m_options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);

        // Setup vertex buffer description.
        SDL_GPUVertexBufferDescription vb_desc{};
        vb_desc.slot = 0;
        vb_desc.pitch = m_vertex_layout.get_stride();
        vb_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vb_desc.instance_step_rate = 0;
        m_vertex_buffer_descriptions.push_back(vb_desc);


        // Setup color target blend state.
        SDL_GPUColorTargetBlendState blend{};
        blend.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        blend.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend.color_blend_op = SDL_GPU_BLENDOP_ADD;
        blend.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        blend.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        blend.enable_blend = true;

        SDL_GPUColorTargetDescription color_target{};
        color_target.format = swapchain_format;
        color_target.blend_state = blend;

        m_color_target_descriptions.push_back(color_target);

        // Initialize pipeline create info with default state.
        SDL_GPUGraphicsPipelineCreateInfo info{};
        info.vertex_input_state.vertex_buffer_descriptions = m_vertex_buffer_descriptions.data();
        info.vertex_input_state.num_vertex_buffers = static_cast<std::uint32_t>(m_vertex_buffer_descriptions.size());
        info.vertex_input_state.vertex_attributes = m_vertex_layout.get_vertex_attributes().data();
        info.vertex_input_state.num_vertex_attributes =
            static_cast<std::uint32_t>(m_vertex_layout.get_vertex_attributes().size());

        info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

        info.target_info.color_target_descriptions = m_color_target_descriptions.data();
        info.target_info.num_color_targets = static_cast<std::uint32_t>(m_color_target_descriptions.size());

        // Shaders will be filled in later by set_vertex_shader / set_fragment_shader.
        info.vertex_shader = nullptr;
        info.fragment_shader = nullptr;

        m_pipeline_info = info;
        m_modified = true;
    }

    /**
     * @brief Releases owned GPU pipeline and shader resources and clears their handles.
     *
     * If a GPU wrapper is available, releases the graphics pipeline, vertex shader,
     * and fragment shader held by this object (if present) and resets their pointers
     * to nullptr.
     */
    RenderProcedural::~RenderProcedural() noexcept
    {
        if (!m_gpu)
        {
            return;
        }

        if (m_graphics_pipeline)
        {
            m_gpu->release_pipeline(m_graphics_pipeline);
            m_graphics_pipeline = nullptr;
        }

        if (m_vertex_shader)
        {
            m_gpu->release_shader(m_vertex_shader);
            m_vertex_shader = nullptr;
        }

        if (m_fragment_shader)
        {
            m_gpu->release_shader(m_fragment_shader);
            m_fragment_shader = nullptr;
        }
    }

    /**
     * @brief Ensures the GPU graphics pipeline matches the current pipeline description, creating or replacing the
     * pipeline if changes are pending.
     *
     * If no modifications are pending this function is a no-op. On success it updates the stored graphics pipeline and
     * clears the modified flag.
     *
     * @return std::monostate on success; `std::unexpected<GpuError>` containing the GPU error if pipeline creation
     * fails.
     */
    [[nodiscard]] std::expected<std::monostate, GpuError> RenderProcedural::submit()
    {
        if (!m_modified)
        {
            // Nothing changed, nothing to do.
            return std::monostate{};
        }

        // Refresh pointers in case the underlying vectors were reallocated.
        m_pipeline_info.vertex_input_state.vertex_buffer_descriptions = m_vertex_buffer_descriptions.data();
        m_pipeline_info.vertex_input_state.num_vertex_buffers =
            static_cast<std::uint32_t>(m_vertex_buffer_descriptions.size());

        m_pipeline_info.vertex_input_state.vertex_attributes = m_vertex_layout.get_vertex_attributes().data();
        m_pipeline_info.vertex_input_state.num_vertex_attributes =
            static_cast<std::uint32_t>(m_vertex_layout.get_vertex_attributes().size());

        m_pipeline_info.target_info.color_target_descriptions = m_color_target_descriptions.data();
        m_pipeline_info.target_info.num_color_targets = static_cast<std::uint32_t>(m_color_target_descriptions.size());

        // Attempt to create a new graphics pipeline.
        auto pipeline_result = m_gpu->create_pipeline(m_pipeline_info);
        if (!pipeline_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to create graphics pipeline", __FILE__, __LINE__);
            return std::unexpected(pipeline_result.error());
        }

        SDL_GPUGraphicsPipeline* new_pipeline = pipeline_result.value();

        // Replace previous pipeline if any.
        if (m_graphics_pipeline)
        {
            m_gpu->release_pipeline(m_graphics_pipeline);
        }

        m_graphics_pipeline = new_pipeline;
        m_modified = false;

        return std::monostate{};
    }

    /**
     * Compile the provided GLSL vertex shader source, create a GPU vertex shader, and install it into the pipeline.
     *
     * This updates the stored vertex shader, sets the pipeline's vertex_shader to the new shader, and marks the
     * pipeline as modified so a new graphics pipeline will be created on the next submit.
     *
     * @param source GLSL source code for the vertex shader.
     * @return std::monostate on success; `std::unexpected(GpuError::COMPILE_VERTEX_SHADER_FAILED)` if shader
     * compilation fails, or `std::unexpected(<other GpuError>)` if shader creation on the GPU fails.
     */
    [[nodiscard]] std::expected<std::monostate, GpuError> RenderProcedural::set_vertex_shader(const std::string& source)
    {
        auto result = m_compiler.CompileGlslToSpv(source, shaderc_glsl_vertex_shader, "vertex.glsl", m_options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[shaderc] compile error in vertex.glsl: %s",
                         result.GetErrorMessage().c_str());

            // You may want to add this to your GpuError enum.
            return std::unexpected(GpuError::COMPILE_VERTEX_SHADER_FAILED);
        }

        // Convert SPIR-V words to a byte vector
        std::vector<std::uint8_t> code = spv_result_to_bytes(result);

        auto shader_result = m_gpu->create_shader(code, SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
        if (!shader_result)
        {
            return std::unexpected(shader_result.error());
        }

        SDL_GPUShader* new_shader = shader_result.value();

        // Release previous shader, if any
        if (m_vertex_shader)
        {
            m_gpu->release_shader(m_vertex_shader);
        }

        m_vertex_shader = new_shader;
        m_pipeline_info.vertex_shader = new_shader;
        m_modified = true;

        auto reflect_result = reflect_vertex(source);
        set_vertex_reflection(reflect_result);

        return std::monostate{};
    }

    /**
     * @brief Compiles GLSL fragment shader source, creates a GPU fragment shader, and installs it into the pipeline.
     *
     * Compiles the provided GLSL fragment source to SPIR-V, converts the result into a byte vector, and asks the GPU
     * wrapper to create a fragment shader; on success the new shader replaces any existing fragment shader in the
     * object, updates the pipeline's fragment shader reference, and marks the pipeline state as modified.
     *
     * @param source GLSL source code for the fragment shader.
     * @return std::monostate on success; `std::unexpected<GpuError>` on failure (compilation error or GPU
     * shader-creation error).
     */
    [[nodiscard]] std::expected<std::monostate, GpuError>
    RenderProcedural::set_fragment_shader(const std::string& source)
    {
        auto result = m_compiler.CompileGlslToSpv(source, shaderc_glsl_fragment_shader, "fragment.glsl", m_options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[shaderc] compile error in fragment.glsl: %s",
                         result.GetErrorMessage().c_str());

            // You may want to add this to your GpuError enum.
            return std::unexpected(GpuError::COMPILE_FRAGMENT_SHADER_FAILED);
        }

        // Convert SPIR-V words to a byte vector
        std::vector<std::uint8_t> code = spv_result_to_bytes(result);

        auto shader_result = m_gpu->create_shader(code, SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 1);
        if (!shader_result)
        {
            return std::unexpected(shader_result.error());
        }

        SDL_GPUShader* new_shader = shader_result.value();

        // Release previous shader, if any
        if (m_fragment_shader)
        {
            m_gpu->release_shader(m_fragment_shader);
        }

        m_fragment_shader = new_shader;
        m_pipeline_info.fragment_shader = new_shader;
        m_modified = true;

        return std::monostate{};
    }

} // namespace sopho
