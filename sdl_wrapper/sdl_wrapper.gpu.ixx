// sdl_wrapper.gpu.ixx
// Created by sophomore on 11/12/25.
//
module;
#include <expected>
#include <memory>
#include <variant>

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_video.h"
#include "shaderc/shaderc.hpp"
export module sdl_wrapper:gpu;
import :decl;
import :buffer;
import :pipeline;
export namespace sopho
{

    class GpuWrapper : public std::enable_shared_from_this<GpuWrapper>
    {
        std::expected<SDL_GPUDevice*, GpuError> m_device{};

        // TODO: Consider multi window situation
        std::expected<SDL_Window*, GpuError> m_window{};

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
            m_device.and_then(
                [&](auto device)
                {
                    return m_window.and_then(
                        [&](auto window) -> std::expected<std::monostate, GpuError>
                        {
                            SDL_ClaimWindowForGPUDevice(device, window);
                            return std::monostate{};
                        });
                });
        }

        ~GpuWrapper()
        {
            m_window.and_then(
                [&](auto window)
                {
                    SDL_DestroyWindow(window);
                    return m_device.and_then(
                        [&](auto device) -> std::expected<std::monostate, GpuError>
                        {
                            SDL_ReleaseWindowFromGPUDevice(device, window);
                            return std::monostate{};
                        });
                });
            m_device.and_then(
                [&](auto device) -> std::expected<std::monostate, GpuError>
                {
                    SDL_DestroyGPUDevice(device);
                    return std::monostate{};
                });
        }

        auto data() { return m_device; }

        std::expected<BufferWrapper, GpuError> create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size);

        auto release_buffer(SDL_GPUBuffer* buffer)
        {
            m_device.and_then(
                [&](auto device) -> std::expected<std::monostate, GpuError>
                {
                    SDL_ReleaseGPUBuffer(device, buffer);
                    return std::monostate{};
                });
        }

        std::expected<PipelineWrapper, GpuError> create_pipeline_wrapper();

        auto create_pipeline(const std::expected<SDL_GPUGraphicsPipelineCreateInfo, GpuError>& create_info)
        {
            return m_device.and_then(
                [&](auto device)
                {
                    return create_info.and_then(
                        [&](auto pipeline_info) -> std::expected<SDL_GPUGraphicsPipeline*, GpuError>
                        {
                            auto pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
                            if (!pipeline)
                            {
                                return std::unexpected(GpuError::CREATE_PIPELINE_FAILED);
                            }
                            return pipeline;
                        });
                });
        }

        auto create_shader(const std::vector<uint8_t>& shader, SDL_GPUShaderStage stage, uint32_t num_uniform_buffers)
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
                [&](auto device)
                {
                    return shader_expected.and_then(
                        [&](auto shader) -> std::expected<std::monostate, GpuError>
                        {
                            SDL_ReleaseGPUShader(device, shader);
                            return std::monostate{};
                        });
                });
        }

        auto release_pipeline(SDL_GPUGraphicsPipeline* graphics_pipeline)
        {
            m_device.and_then(
                [&](auto device) -> std::expected<std::monostate, GpuError>
                {
                    SDL_ReleaseGPUGraphicsPipeline(device, graphics_pipeline);
                    return std::monostate{};
                });
        }

        auto acquire_window() { return m_window; }

        auto get_texture_format()
        {
            return m_device.and_then(
                [&](auto device) -> std::expected<SDL_GPUTextureFormat, GpuError>
                {
                    return m_window.and_then(
                        [&](auto window) -> std::expected<SDL_GPUTextureFormat, GpuError>
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
