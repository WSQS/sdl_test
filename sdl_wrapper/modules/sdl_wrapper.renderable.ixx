// sdl_wrapper.renderable.ixx
// Created by wsqsy on 11/19/2025.
//
module;
#include <SDL3/SDL_gpu.h>
#include <array>
#include <memory>
#include <variant>
export module sdl_wrapper:renderable;
import logos;
import data_type;
import primitive_renderer;
import :decl;
namespace sopho
{

    export struct RenderContext
    {
        SDL_GPURenderPass* render_pass{};
        SDL_GPUCommandBuffer* command_buffer{};
        std::array<Mat<float, 4, 4>, 3> camera_mat{};
        std::array<Mat<float, 1, 4>, 2> pos{};
        std::shared_ptr<TextureWrapper> texture_wrapper{};
    };

    export class Renderable
    {
    public:
        RenderProcedureHandle m_render_procedural{};
        std::shared_ptr<RenderData> m_render_data{};
        auto& procedural() { return m_render_procedural; }
        std::shared_ptr<RenderData>& data() { return m_render_data; }
        checkable<std::monostate> draw(RenderContext render_contex);
    };
} // namespace sopho
