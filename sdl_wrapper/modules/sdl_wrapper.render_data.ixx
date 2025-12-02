// sdl_wrapper.render_data.ixx
// Created by sophomore on 11/18/25.
//
module;
#include <SDL3/SDL_gpu.h>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <variant>
#include <vector>
export module sdl_wrapper:render_data;
import :vertex_layout;
import :gpu;
namespace sopho
{
    export class RenderData
    {
    public:
        struct Builder
        {
            VertexLayout layout{};
            std::uint32_t vertex_count{};
            std::uint32_t index_count{};
            std::vector<std::byte> vertex_data{};
            std::vector<std::byte> index_data{};

            Builder& set_vertex_layout(VertexLayout new_layout)
            {
                layout = new_layout;
                return *this;
            }

            Builder& set_vertex_count(std::uint32_t new_vertex_count)
            {
                vertex_count = new_vertex_count;
                return *this;
            }

            Builder& set_index_count(std::uint32_t new_index_count)
            {
                index_count = new_index_count;
                return *this;
            }

            template <typename VertexType>
            Builder& set_vertices(std::span<VertexType> vertices)
            {
                static_assert(std::is_trivially_copyable_v<VertexType>, "VertexType must be trivially copyable");
                vertex_data =
                    std::vector<std::byte>(reinterpret_cast<const std::byte*>(vertices.data()),
                                           reinterpret_cast<const std::byte*>(vertices.data()) + vertices.size_bytes());
                return *this;
            }
            Builder& set_indices(std::span<const uint32_t> indices)
            {
                index_data =
                    std::vector<std::byte>(reinterpret_cast<const std::byte*>(indices.data()),
                                           reinterpret_cast<const std::byte*>(indices.data()) + indices.size_bytes());
                return *this;
            }

            checkable<std::shared_ptr<RenderData>> build(GpuWrapper& gpu);
        };
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
