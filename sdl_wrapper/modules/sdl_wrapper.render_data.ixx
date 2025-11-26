// sdl_wrapper.render_data.ixx
// Created by sophomore on 11/18/25.
//
module;
#include <SDL3/SDL_gpu.h>
#include <cstddef>
#include <expected>
#include <variant>
#include <vector>
export module sdl_wrapper:render_data;
import :vertex_layout;
namespace sopho
{
    export class RenderData
    {
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
        virtual ~RenderData() = default;
        virtual std::vector<SDL_GPUBufferBinding>& get_vertex_buffer_binding() = 0;
        virtual SDL_GPUBufferBinding& get_index_buffer_binding() = 0;
        virtual VertexView vertex_view() = 0;
        virtual IndexView index_view() = 0;
        virtual std::expected<std::monostate, GpuError> upload() = 0;
    };
} // namespace sopho
