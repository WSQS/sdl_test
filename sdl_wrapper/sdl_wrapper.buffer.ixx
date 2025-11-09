//
// Created by sophomore on 11/8/25.
//
module;
#include "SDL3/SDL_gpu.h"
export module sdl_wrapper:buffer;

namespace sopho {
    export class BufferWrapper {
        SDL_GPUBuffer *vertexBuffer{};
    };
}
