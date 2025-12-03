// primitive_renderer.ixx
// Created by wsqsy on 12/3/2025.
//
module;
#include <string>
#include <cstdint>
export module primitive_renderer;
import data_type;

export namespace sopho
{
    enum class RenderProcedureHandle : std::uint32_t
    {
    };
    struct RenderProcedureDescriptor
    {
        std::string vert_shader{};
        std::string frag_shader{};
    };
    enum class TextureHandle : std::uint32_t
    {
    };
    struct RenderPassDescriptor
    {
        bool clear{};
        bool depth{};
    };
    class PrimitiveRenderer
    {
    public:
        virtual ~PrimitiveRenderer() = default;
        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
        virtual void begin_render_pass(const RenderPassDescriptor& render_pass_descriptor) = 0;
        virtual void end_render_pass() = 0;
        virtual checkable<RenderProcedureHandle>
        create_render_procedure(const RenderProcedureDescriptor& render_procedure_descriptor) = 0;
        virtual void bind_render_procedure(const RenderProcedureHandle& render_procedure_handle) = 0;
    };
} // namespace sopho
