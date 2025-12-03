// sdl_wrapper.renderable.cpp
// Created by wsqsy on 12/1/2025.
//
module;
#include <SDL3/SDL_gpu.h>
#include <array>
#include <expected>
#include <variant>
module sdl_wrapper;
import logos;
import :renderable;
import :render_procedural;
import :render_data;
namespace sopho
{
    checkable<std::monostate> Renderable::draw(RenderContext render_contex)
    {
        if (render_contex.texture_wrapper)
        {
            SDL_BindGPUFragmentSamplers(render_contex.render_pass, 0, render_contex.texture_wrapper->get(), 1);
        }
        SDL_DrawGPUIndexedPrimitives(render_contex.render_pass, data()->index_view().index_count, 1, 0, 0, 0);
        return std::monostate{};
    }
} // namespace sopho
