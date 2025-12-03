// sdl_primitive_renderer.ixx
// Created by wsqsy on 12/3/2025.
//
module;
#include <expected>
#include <memory>

#include "SDL3/SDL_gpu.h"
export module sdl_primitive_renderer;
import data_type;
import primitive_renderer;
import sdl_raii;
import sdl_wrapper;
namespace sopho
{
    export class SDLPrimitiveRenderer : public PrimitiveRenderer
    {
        std::shared_ptr<GpuWrapper> m_gpu;
        SDLPrimitiveRenderer(std::shared_ptr<GpuWrapper> gpu) : m_gpu(std::move(gpu)) {}
        GpuCommandBufferRaii m_gpu_command_buffer;

    public:
        static checkable<SDLPrimitiveRenderer*> create(std::shared_ptr<GpuWrapper> gpu_wrapper)
        {
            return new SDLPrimitiveRenderer{gpu_wrapper};
        }
        ~SDLPrimitiveRenderer() override = default;

        void begin_frame() override
        {
            auto p_buffer = SDL_AcquireGPUCommandBuffer(m_gpu->device());
            m_gpu_command_buffer.reset(p_buffer);
        }
        void end_frame() override { m_gpu_command_buffer.reset(); }
        auto& get_gpu()
        {
            return *m_gpu;
        }
    };
} // namespace sopho
