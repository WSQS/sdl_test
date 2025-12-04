// renderer_factory.ixx
// Created by wsqsy on 12/3/2025.
//
module;
#include <string>
export module renderer_factory;
import data_type;
import primitive_renderer;
import sdl_primitive_renderer;
import sdl_gl_primitive_renderer;
namespace sopho
{
    export checkable<PrimitiveRenderer*> create_primitive_renderer(const std::string& renderer_name)
    {
        if (renderer_name == "SDL")
        {
            return SDLPrimitiveRenderer::create();
        }
        else
        {
            return SDLGLPrimitiveRenderer::create();
        }
    }
} // namespace sopho
