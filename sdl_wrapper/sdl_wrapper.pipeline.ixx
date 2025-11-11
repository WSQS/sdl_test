//
// Created by sophomore on 11/11/25.
//
module;
#include "SDL3/SDL_gpu.h"
export module sdl_wrapper:pipeline;

namespace sopho {
    class PileLineWrapper {
        SDL_GPUGraphicsPipeline *m_graphics_pipeline{};

    public:
        PileLineWrapper() = default;

        auto data() {
            return m_graphics_pipeline;
        }
    };
}
