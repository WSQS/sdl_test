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
        std::vector<SDL_GPUBufferBinding> m_bindings{};

    public:
        explicit RenderData(BufferWrapper&& buffer_wrapper, const VertexLayout& layouts) :
            m_buffer(std::move(buffer_wrapper)), m_layouts(layouts)
        {
            m_bindings.emplace_back(m_buffer.gpu_buffer(), 0);
        }

    public:
        RenderData(const RenderData&) = delete;
        RenderData& operator=(const RenderData&) = delete;
        RenderData(RenderData&&) = default;
        RenderData& operator=(RenderData&&) = default;
        auto& buffer() { return m_buffer; }
        auto& get_buffer_binding() { return m_bindings; }
    };
} // namespace sopho
