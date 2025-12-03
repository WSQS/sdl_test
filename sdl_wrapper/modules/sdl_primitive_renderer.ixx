// sdl_primitive_renderer.ixx
// Created by wsqsy on 12/3/2025.
//
module;
#include <expected>
#include <map>
#include <memory>

#include "SDL3/SDL_gpu.h"
export module sdl_primitive_renderer;
import data_type;
import primitive_renderer;
import sdl_raii;
import sdl_wrapper;
namespace sopho
{
    export class SDLPrimitiveRenderer : public PrimitiveRenderer
    {
        std::shared_ptr<GpuWrapper> m_gpu;
        SDLPrimitiveRenderer(std::shared_ptr<GpuWrapper> gpu) : m_gpu(std::move(gpu)) {}
        GpuCommandBufferRaii m_gpu_command_buffer;
        std::map<RenderProcedureHandle, RenderProcedural> m_render_procedures;

    public:
        static checkable<SDLPrimitiveRenderer*> create()
        {
            auto gpu_result = GpuWrapper::create();
            if (!gpu_result)
            {
                return std::unexpected(gpu_result.error());
            }
            return new SDLPrimitiveRenderer{gpu_result.value()};
        }
        ~SDLPrimitiveRenderer() override = default;

        void begin_frame() override
        {
            auto p_buffer = SDL_AcquireGPUCommandBuffer(m_gpu->device());
            m_gpu_command_buffer.reset(p_buffer);
        }
        void end_frame() override { m_gpu_command_buffer.reset(); }
        checkable<RenderProcedureHandle>
        create_render_procedure(const RenderProcedureDescriptor& render_procedure_descriptor) override
        {
            auto pw_result = get_gpu().create_render_procedural();
            if (!pw_result)
            {
                return std::unexpected(pw_result.error());
            }

            auto pipeline_init =
                pw_result
                    .and_then([&](auto& pipeline)
                              { return pipeline.set_vertex_shader(render_procedure_descriptor.vert_shader); })
                    .and_then([&](std::monostate)
                              { return pw_result->set_fragment_shader(render_procedure_descriptor.frag_shader); })
                    .and_then([&](std::monostate) { return pw_result->submit(); });

            if (!pipeline_init)
            {
                return std::unexpected(pipeline_init.error());
            }
            RenderProcedureHandle handle{};
            if (!m_render_procedures.empty())
            {
                handle = static_cast<RenderProcedureHandle>(static_cast<std::uint32_t>(m_render_procedures.rbegin()->first) + 1);
            }
            return handle;
        }
        auto& get_gpu() { return *m_gpu; }
    };
} // namespace sopho
