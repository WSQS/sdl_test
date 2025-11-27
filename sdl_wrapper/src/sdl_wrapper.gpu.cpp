// sdl_wrapper.gpu.cpp
// Created by wsqsy on 11/14/2025.
//
module;
#include <expected>
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
module sdl_wrapper;
import :gpu;
import :render_procedural;
import :buffer;
import :render_data;
import :vertex_layout;
import :transfer_buffer;
import :render_data_impl;
namespace sopho
{

    /**
     * @brief Create a RenderProcedural configured for the device's texture format.
     *
     * Queries the GPU's texture format and constructs a RenderProcedural associated with this GpuWrapper.
     *
     * @return std::expected<RenderProcedural, GpuError> Contains the constructed RenderProcedural on success, or an
     * unexpected holding the corresponding GpuError on failure.
     */
    std::expected<RenderProcedural, GpuError> GpuWrapper::create_render_procedural()
    {
        // Query texture format, then construct RenderProcedural
        return get_texture_format().transform([self = shared_from_this()](SDL_GPUTextureFormat format)
                                              { return RenderProcedural{self, format}; });
    }
} // namespace sopho
