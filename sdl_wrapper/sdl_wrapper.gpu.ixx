// sdl_wrapper.gpu.ixx
// Created by sophomore on 11/12/25.
//
module;
#include <expected>
#include <memory>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_video.h"
#include "shaderc/shaderc.hpp"
export module sdl_wrapper:gpu;
import :buffer;
import :pipeline;
export namespace sopho
{

    enum class GpuError
    {
        CREATE_DEVICE_FAILED,
        CREATE_WINDOW_FAILED,
        CREATE_BUFFER_FAILED,
        CREATE_SHADER_FAILED,
        GET_TEXTUREFORMAT_FAILED,
    };

    class GpuWrapper : public std::enable_shared_from_this<GpuWrapper>
    {
        std::expected<SDL_GPUDevice*, GpuError> m_device{};

        // TODO: Consider multi window situation
        std::expected<SDL_Window*,GpuError> m_window{};

    public:
        GpuWrapper()
        {
            m_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
            if (!m_device)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                m_device = std::unexpected(GpuError::CREATE_DEVICE_FAILED);
            }
            m_window = SDL_CreateWindow("Hello, Triangle!", 960, 540, SDL_WINDOW_RESIZABLE);
            if (!m_window)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                m_window = std::unexpected(GpuError::CREATE_WINDOW_FAILED);
            }
        }

        ~GpuWrapper()
        {
            m_window.and_then([&](auto window)
            {
                m_device.and_then([&](auto device)
                {
                SDL_ReleaseWindowFromGPUDevice(window, device);
                });
                SDL_DestroyWindow(window);
            });
            m_device.and_then([&](auto device)
            {
                SDL_DestroyGPUDevice(device);
            });

        }

        auto data() { return m_device; }

        std::expected<BufferWrapper, GpuError> create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size);

        std::expected<PipelineWrapper,GpuError> create_pipeline();

        auto create_shader(const std::vector<uint8_t>& shader, SDL_GPUShaderStage stage,
                           uint32_t num_uniform_buffers)
        {
            return m_device.and_then(
                [&](auto p_device) -> std::expected<SDL_GPUShader*, GpuError>
                {
                    SDL_GPUShaderCreateInfo info{};
                    info.code = shader.data();
                    info.code_size = shader.size();
                    info.entrypoint = "main";
                    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
                    info.stage = stage;
                    info.num_samplers = 0;
                    info.num_storage_buffers = 0;
                    info.num_storage_textures = 0;
                    info.num_uniform_buffers = num_uniform_buffers;
                    auto shader = SDL_CreateGPUShader(p_device, &info);
                    if (!shader)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                        return std::unexpected(GpuError::CREATE_SHADER_FAILED);
                    }
                    return shader;
                });
        }

        auto release_shader(const std::expected<SDL_GPUShader*, GpuError>& shader_expected)
        {
            m_device.and_then(
                [&](auto p_device)
                { shader_expected.and_then([&](auto shader) { SDL_ReleaseGPUShader(p_device, shader); }); });
        }

        auto acquire_window()
        {
            return m_window;
        }

        auto get_texture_format()
        {
            return m_device.and_then([&](auto device) -> std::expected<SDL_GPUTextureFormat, GpuError>
            {
                return m_window.and_then([&](auto window)
                {
                    auto format = SDL_GetGPUSwapchainTextureFormat(device, window);
                    if (format == SDL_GPU_TEXTUREFORMAT_INVALID)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                        return std::unexpected(GpuError::GET_TEXTUREFORMAT_FAILED);
                    }
                    return format;
                });
            });
        }
    };
} // namespace sopho
