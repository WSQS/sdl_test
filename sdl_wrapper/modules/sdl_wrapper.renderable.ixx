// sdl_wrapper.renderable.ixx
// Created by wsqsy on 11/19/2025.
//
module;
#include <SDL3/SDL_gpu.h>
#include <array>
#include <memory>
#include <variant>
export module sdl_wrapper:renderable;
import data_type;
import primitive_renderer;
import :decl;
namespace sopho
{
    export class Renderable
    {
    public:
        RenderProcedureHandle m_render_procedural{};
        std::shared_ptr<RenderData> m_render_data{};
        auto& procedural() { return m_render_procedural; }
        std::shared_ptr<RenderData>& data() { return m_render_data; }
    };
} // namespace sopho
