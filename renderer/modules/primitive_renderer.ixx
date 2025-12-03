// primitive_renderer.ixx
// Created by wsqsy on 12/3/2025.
//
module;
#include <cstdint>
#include <span>
#include <string>
#include <vector>
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
    enum class BufferHandle : std::uint32_t
    {
    };
    enum class BufferUsage : std::uint32_t
    {
        VERTEX,
        INDEX,
    };
    struct BufferDescriptor
    {
        BufferUsage buffer_usage{};
        std::vector<std::byte> data{};
    };
    struct RenderPassDescriptor
    {
        bool clear{};
        bool depth{};
    };
    struct UniformDescriptor
    {
        std::int32_t slot_index{};
        std::span<std::byte> data{};
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
        virtual checkable<BufferHandle> create_buffer(const BufferDescriptor& buffer_descriptor) = 0;
        virtual void bind_vertex_buffer(const BufferHandle& buffer_handle) = 0;
        virtual void bind_index_buffer(const BufferHandle& buffer_handle) = 0;
        virtual void push_vertex_uniform(const UniformDescriptor& uniform_descriptor) = 0;
        virtual void push_fragment_uniform(const UniformDescriptor& uniform_descriptor) = 0;
        virtual void draw_index(std::int32_t num_indices) = 0;
    };
} // namespace sopho
