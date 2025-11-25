// sdl_wrapper.render_data_impl.ixx
// Created by wsqsy on 11/25/2025.
//
module;
#include <utility>
export module sdl_wrapper:render_data_impl;
import :render_data;
import :buffer;
namespace sopho
{
    class RenderDataImpl : public RenderData
    {
        BufferWrapper m_vertex_buffer;
        BufferWrapper m_index_buffer;
        VertexLayout m_layouts{};
        size_t m_vertex_count{};
        std::vector<SDL_GPUBufferBinding> m_bindings{};
        SDL_GPUBufferBinding m_index_binding{};

    public:
        explicit RenderDataImpl(BufferWrapper&& vertex_buffer_wrapper, BufferWrapper&& index_buffer_wrapper,
                                const VertexLayout& layouts, size_t vertex_count) :
            m_vertex_buffer(std::move(vertex_buffer_wrapper)), m_index_buffer(std::move(index_buffer_wrapper)),
            m_layouts(layouts), m_vertex_count(vertex_count)
        {
            m_bindings.emplace_back(m_vertex_buffer.gpu_buffer(), 0);
            m_index_binding.buffer = m_index_buffer.gpu_buffer();
            m_index_binding.offset = 0;
        }
        RenderDataImpl(const RenderDataImpl&) = delete;
        RenderDataImpl& operator=(const RenderDataImpl&) = delete;
        RenderDataImpl(RenderDataImpl&&) = default;
        RenderDataImpl& operator=(RenderDataImpl&&) = default;
        virtual std::vector<SDL_GPUBufferBinding>& get_vertex_buffer_binding() override { return m_bindings; }
        virtual SDL_GPUBufferBinding& get_index_buffer_binding() override { return m_index_binding; }
        virtual VertexView vertex_view() override
        {
            return VertexView{.layout = m_layouts, .vertex_count = m_vertex_count, .raw = m_vertex_buffer.cpu_buffer()};
        }
        virtual IndexView index_view() override
        {
            return IndexView{.index_count = m_vertex_count, .raw = m_index_buffer.cpu_buffer()};
        }
        virtual std::expected<std::monostate, GpuError> upload() override
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
