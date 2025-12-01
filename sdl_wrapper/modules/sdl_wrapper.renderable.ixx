// sdl_wrapper.renderable.ixx
// Created by wsqsy on 11/19/2025.
//
module;
#include <SDL3/SDL_gpu.h>
#include <memory>
export module sdl_wrapper:renderable;
import logos;
import data_type;
import :decl;
namespace sopho
{

    export struct RenderContex
    {
        SDL_GPURenderPass* render_pass{};
        SDL_GPUCommandBuffer* command_buffer{};
        Mat<float, 4, 4> camera_mat{};
        std::shared_ptr<TextureWrapper> texture_wrapper{};
    };

    export class Renderable
    {
    public:
        std::shared_ptr<RenderProcedural> m_render_procedural{};
        std::shared_ptr<RenderData> m_render_data{};
        auto& procedural() { return m_render_procedural; }
        std::shared_ptr<RenderData>& data() { return m_render_data; }
        checkable<std::monostate> draw(RenderContex render_contex);
    };
} // namespace sopho
