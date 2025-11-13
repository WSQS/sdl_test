//
// Created by sophomore on 11/12/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
export module sdl_wrapper:gpu;
import :buffer;
namespace sopho
{
    export class GpuWrapper : public std::enable_shared_from_this<GpuWrapper>
    {
        SDL_GPUDevice* m_device{};

    public:
        GpuWrapper() : m_device(SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr))
        {
            if (m_device == nullptr)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            }
        }

        ~GpuWrapper()
        {
            if (m_device)
            {
                SDL_DestroyGPUDevice(m_device);
            }
            m_device = nullptr;
        }

        auto data() { return m_device; }

        auto create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size)
        {
            SDL_GPUBufferCreateInfo create_info{flag, size};
            auto buffer = SDL_CreateGPUBuffer(m_device, &create_info);
            BufferWrapper result(shared_from_this(), buffer);
            return result;
        }
    };
} // namespace sopho
