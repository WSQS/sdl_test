//
// Created by sophomore on 11/12/25.
//
module;
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_video.h"
#include "shaderc/shaderc.hpp"
export module sdl_wrapper:gpu;
import :buffer;
import :pipeline;
import :window;
namespace sopho
export namespace sopho
{
    export class GpuWrapper : public std::enable_shared_from_this<GpuWrapper>
    {
        SDL_GPUDevice* m_device{};

        SDL_Window* m_window{};

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

        BufferWrapper create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size);

        PipelineWrapper create_pipeline();

        auto create_shader(const std::vector<uint8_t>& p_shader, SDL_GPUShaderStage p_stage)
        {
            SDL_GPUShaderCreateInfo vertexInfo{};
            vertexInfo.code = p_shader.data();
            vertexInfo.code_size = p_shader.size();
            vertexInfo.entrypoint = "main";
            vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
            vertexInfo.stage = p_stage;
            vertexInfo.num_samplers = 0;
            vertexInfo.num_storage_buffers = 0;
            vertexInfo.num_storage_textures = 0;
            vertexInfo.num_uniform_buffers = 0;
            return SDL_CreateGPUShader(m_device, &vertexInfo);
        }

        auto release_shader(SDL_GPUShader* shader)
        {
            if (shader)
            {
                SDL_ReleaseGPUShader(m_device, shader);
            }
        }

        auto get_texture_format()
        {
            return SDL_GetGPUSwapchainTextureFormat(m_device, m_window);
        }

        auto create_window()
        {
            auto window = SDL_CreateWindow("Hello, Triangle!", 960, 540, SDL_WINDOW_RESIZABLE);
            m_window = window;
            SDL_ClaimWindowForGPUDevice(m_device, window);
            return WindowWrapper{shared_from_this(),window};
        }

        auto release_window(SDL_Window* p_window)
        {
            if (p_window)
            {
                SDL_ReleaseWindowFromGPUDevice(m_device,p_window);
                SDL_DestroyWindow(p_window);
            }
        }

        auto set_window(SDL_Window* p_window)
        {
            m_window = p_window;
        }
    };
} // namespace sopho
