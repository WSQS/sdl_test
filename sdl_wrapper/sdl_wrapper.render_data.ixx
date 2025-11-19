// sdl_wrapper.render_data.ixx
// Created by sophomore on 11/18/25.
//
module;
#include <SDL3/SDL_gpu.h>
#include <utility>
#include <vector>
export module sdl_wrapper:render_data;
import :buffer;
import :vertex_layout;
namespace sopho
{
    export class RenderData
    {
        BufferWrapper m_buffer;
        VertexLayout m_layouts{};
        size_t m_vertex_count{};
        std::vector<SDL_GPUBufferBinding> m_bindings{};

    public:
        explicit RenderData(BufferWrapper&& buffer_wrapper, const VertexLayout& layouts, size_t vertex_count) :
            m_buffer(std::move(buffer_wrapper)), m_layouts(layouts), m_vertex_count(vertex_count)
        {
            m_bindings.emplace_back(m_buffer.gpu_buffer(), 0);
        }

    public:
        struct VertexView
        {
            VertexLayout layout{};
            size_t vertex_count{};
            std::byte* raw{};
        };
        RenderData(const RenderData&) = delete;
        RenderData& operator=(const RenderData&) = delete;
        RenderData(RenderData&&) = default;
        RenderData& operator=(RenderData&&) = default;
        auto& get_buffer_binding() { return m_bindings; }
        auto vertex_view()
        {
            return VertexView{.layout = m_layouts, .vertex_count = m_vertex_count, .raw = m_buffer.cpu_buffer()};
        }
        auto upload()
        {
            return m_buffer.upload();
        }
    };
} // namespace sopho
