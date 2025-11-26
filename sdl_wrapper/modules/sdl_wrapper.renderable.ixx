// sdl_wrapper.renderable.ixx
// Created by wsqsy on 11/19/2025.
//
module;
#include <memory>
export module sdl_wrapper:renderable;
import :decl;
namespace sopho
{
    export class Renderable
    {
    public:
        std::shared_ptr<RenderProcedural> m_render_procedural{};
        std::shared_ptr<RenderData> m_render_data{};
        auto & procedural()
        {
            return m_render_procedural;
        }
        std::shared_ptr<RenderData> & data()
        {
            return m_render_data;
        }
    };
}
