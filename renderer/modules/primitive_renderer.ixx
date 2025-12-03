// primitive_renderer.ixx
// Created by wsqsy on 12/3/2025.
//
module;
#include <string>
export module primitive_renderer;
import data_type;

export namespace sopho
{
    enum class RenderProcedureHandle : std::uint32_t {};
    struct RenderProcedureDescriptor
    {
        std::string vert_shader{};
        std::string frag_shader{};
    };
    class PrimitiveRenderer
    {
    public:
        virtual ~PrimitiveRenderer() = default;
        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
        virtual checkable<RenderProcedureHandle> create_render_procedure(const RenderProcedureDescriptor & render_procedure_descriptor) = 0;
    };
} // namespace sopho
