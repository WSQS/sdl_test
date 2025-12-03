// renderer_factory.ixx
// Created by wsqsy on 12/3/2025.
//

export module renderer_factory;
import data_type;
import primitive_renderer;
import sdl_primitive_renderer;
namespace sopho
{
    export checkable<PrimitiveRenderer*> create_primitive_renderer()
    {
        return SDLPrimitiveRenderer::create();
    }
}