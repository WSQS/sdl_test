// sdl_wrapper.renderable.cpp
// Created by wsqsy on 12/1/2025.
//
module;
#include <SDL3/SDL_gpu.h>
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
        auto submit_result = procedural()->submit();
        if (!submit_result)
        {
            return std::unexpected(submit_result.error());
        }
        SDL_BindGPUGraphicsPipeline(render_contex.render_pass, procedural()->raw());
        SDL_PushGPUVertexUniformData(render_contex.command_buffer, 0, render_contex.camera_mat.data(),
                                     sizeof(Mat<float, 4, 4>));
        SDL_PushGPUFragmentUniformData(render_contex.command_buffer, 0, Mat<float,1,3>{1.,0.5,0.5}.data(),
                                     sizeof(Mat<float, 1, 3>));
        SDL_BindGPUVertexBuffers(render_contex.render_pass, 0, data()->get_vertex_buffer_binding().data(),
                                 data()->get_vertex_buffer_binding().size());

        SDL_BindGPUIndexBuffer(render_contex.render_pass, &data()->get_index_buffer_binding(),
                               SDL_GPU_INDEXELEMENTSIZE_32BIT);
        if (render_contex.texture_wrapper)
        {
            SDL_BindGPUFragmentSamplers(render_contex.render_pass, 0, render_contex.texture_wrapper->get(), 1);
        }
        SDL_DrawGPUIndexedPrimitives(render_contex.render_pass, data()->index_view().index_count, 1, 0, 0, 0);
        return std::monostate{};
    }
} // namespace sopho
