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
        BufferWrapper m_vertex_buffer;
        BufferWrapper m_index_buffer;
        VertexLayout m_layouts{};
        size_t m_vertex_count{};
        std::vector<SDL_GPUBufferBinding> m_bindings{};
        SDL_GPUBufferBinding m_index_binding{};

    public:
        explicit RenderData(BufferWrapper&& vertex_buffer_wrapper, BufferWrapper&& index_buffer_wrapper,
                            const VertexLayout& layouts, size_t vertex_count) :
            m_vertex_buffer(std::move(vertex_buffer_wrapper)), m_index_buffer(std::move(index_buffer_wrapper)),
            m_layouts(layouts), m_vertex_count(vertex_count)
        {
            m_bindings.emplace_back(m_vertex_buffer.gpu_buffer(), 0);
            m_index_binding.buffer = m_index_buffer.gpu_buffer();
            m_index_binding.offset = 0;
        }

    public:
        struct VertexView
        {
            VertexLayout layout{};
            size_t vertex_count{};
            std::byte* raw{};
        };
        struct IndexView
        {
            size_t index_count{};
            std::byte* raw{};
        };
        RenderData(const RenderData&) = delete;
        RenderData& operator=(const RenderData&) = delete;
        RenderData(RenderData&&) = default;
        RenderData& operator=(RenderData&&) = default;
        auto& buffer() { return m_vertex_buffer; }
        auto& get_vertex_buffer_binding() { return m_bindings; }
        auto& get_index_buffer_binding() { return m_index_binding; }
        auto vertex_view()
        {
            return VertexView{.layout = m_layouts, .vertex_count = m_vertex_count, .raw = m_vertex_buffer.cpu_buffer()};
        }
        auto index_view() { return IndexView{.index_count = m_vertex_count, .raw = m_index_buffer.cpu_buffer()}; }
        auto upload()
        {
            auto result = m_vertex_buffer.upload();
            if (!result)
            {
                return result;
            }
            result = m_index_buffer.upload();
            return result;
        }
    };
} // namespace sopho
