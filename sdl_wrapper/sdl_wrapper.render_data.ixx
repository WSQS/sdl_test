// sdl_wrapper.render_data.ixx
// Created by sophomore on 11/18/25.
//
module;
#include <utility>
export module sdl_wrapper:render_data;
import :buffer;
namespace sopho
{
    export class RenderData
    {
        BufferWrapper m_buffer;
    public:
        explicit RenderData(BufferWrapper&& buffer_wrapper) : m_buffer(std::move(buffer_wrapper)) {}
    public:
        RenderData(const RenderData&) = delete;
        RenderData& operator=(const RenderData&) = delete;
        RenderData(RenderData&&) = default;
        RenderData& operator=(RenderData&&) = default;
        auto& buffer()
        {
            return m_buffer;
        }
    };
} // namespace sopho
