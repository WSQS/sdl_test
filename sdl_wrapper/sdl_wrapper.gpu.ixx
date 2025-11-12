//
// Created by sophomore on 11/12/25.
//
module;
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
export module sdl_wrapper:gpu;

namespace sopho {
    export class GpuWrapper {
        SDL_GPUDevice *m_device{};

    public:
        GpuWrapper() : m_device(SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL)) {
            if (m_device == nullptr) {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            }
        }

        ~GpuWrapper() {
            if (m_device)
            {
                SDL_DestroyGPUDevice(m_device);
            }
            m_device = nullptr;
        }

        auto data() {
            return m_device;
        }
    };
}
