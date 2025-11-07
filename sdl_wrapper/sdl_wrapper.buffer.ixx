//
// Created by sophomore on 11/8/25.
//

export module sdl_wrapper:buffer;
#include "SDL3/SDL_gpu.h"

namespace sopho {
    export class BufferWrapper {
        SDL_GPUBuffer *vertexBuffer{};

    };
}
