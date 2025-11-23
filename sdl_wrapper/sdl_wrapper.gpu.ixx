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
import :decl;
import :buffer;
import :render_procedural;
export namespace sopho
{

    struct DeviceHandle
    {
        SDL_GPUDevice* raw{};

        explicit DeviceHandle(SDL_GPUDevice* d) noexcept : raw(d) {}

        DeviceHandle(DeviceHandle&& other) noexcept : raw(other.raw) { other.raw = nullptr; }
        DeviceHandle& operator=(DeviceHandle&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                raw = other.raw;
                other.raw = nullptr;
            }
            return *this;
        }

        DeviceHandle(const DeviceHandle&) = delete;
        DeviceHandle& operator=(const DeviceHandle&) = delete;

        ~DeviceHandle() { reset(); }

        void reset() noexcept
        {
            if (raw)
            {
                SDL_DestroyGPUDevice(raw);
                raw = nullptr;
            }
        }

        [[nodiscard]] bool valid() const noexcept { return raw != nullptr; }
    };

    struct WindowHandle
    {
        SDL_Window* raw{};

        WindowHandle() = default;

        explicit WindowHandle(SDL_Window* w) noexcept : raw(w) {}

        WindowHandle(const WindowHandle&) = delete;
        WindowHandle& operator=(const WindowHandle&) = delete;

        WindowHandle(WindowHandle&& other) noexcept : raw(other.raw) { other.raw = nullptr; }

        WindowHandle& operator=(WindowHandle&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                raw = other.raw;
                other.raw = nullptr;
            }
            return *this;
        }

        ~WindowHandle() noexcept { reset(); }

        void reset() noexcept
        {
            if (raw)
            {
                SDL_DestroyWindow(raw);
                raw = nullptr;
            }
        }

        [[nodiscard]] bool valid() const noexcept { return raw != nullptr; }
    };

    struct ClaimedWindow
    {
        SDL_GPUDevice* device{};
        SDL_Window* window{};

        ClaimedWindow() = default;

        ClaimedWindow(SDL_GPUDevice* d, SDL_Window* w) noexcept : device(d), window(w) {}

        ClaimedWindow(const ClaimedWindow&) = delete;
        ClaimedWindow& operator=(const ClaimedWindow&) = delete;

        ClaimedWindow(ClaimedWindow&& other) noexcept : device(other.device), window(other.window)
        {
            other.device = nullptr;
            other.window = nullptr;
        }

        ClaimedWindow& operator=(ClaimedWindow&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                device = other.device;
                window = other.window;
                other.device = nullptr;
                other.window = nullptr;
            }
            return *this;
        }

        ~ClaimedWindow() noexcept { reset(); }

        void reset() noexcept
        {
            if (device && window)
            {
                SDL_ReleaseWindowFromGPUDevice(device, window);
                device = nullptr;
                window = nullptr;
            }
        }

        [[nodiscard]] bool valid() const noexcept { return device && window; }
    };

    struct GpuContext
    {
        DeviceHandle device;
        WindowHandle window;
        ClaimedWindow claimed;

        GpuContext(DeviceHandle d, WindowHandle w, ClaimedWindow c) noexcept :
            device(std::move(d)), window(std::move(w)), claimed(std::move(c))
        {
        }

        GpuContext(const GpuContext&) = delete;
        GpuContext& operator=(const GpuContext&) = delete;

        GpuContext(GpuContext&&) = default;
        GpuContext& operator=(GpuContext&&) = default;

        [[nodiscard]] SDL_GPUDevice* device_raw() const noexcept { return device.raw; }
        [[nodiscard]] SDL_Window* window_raw() const noexcept { return window.raw; }
    };

    using GpuContextResult = std::expected<GpuContext, GpuError>;

    inline GpuContextResult create_gpu_context()
    {
        auto* dev_raw = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
        if (!dev_raw)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CREATE_DEVICE_FAILED);
        }
        DeviceHandle device{dev_raw};

        auto* win_raw = SDL_CreateWindow("Hello, Triangle!", 960, 540, SDL_WINDOW_RESIZABLE);
        if (!win_raw)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CREATE_WINDOW_FAILED);
        }
        WindowHandle window{win_raw};

        if (!SDL_ClaimWindowForGPUDevice(device.raw, window.raw))
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
            return std::unexpected(GpuError::CLAIM_WINDOW_FAILED);
        }
        ClaimedWindow claimed{device.raw, window.raw};

        return GpuContext{std::move(device), std::move(window), std::move(claimed)};
    }

    class GpuWrapper : public std::enable_shared_from_this<GpuWrapper>
    {

        GpuContext m_ctx;

        explicit GpuWrapper(GpuContext ctx) noexcept : m_ctx(std::move(ctx)) {}

    public:
        [[nodiscard]] static std::expected<std::shared_ptr<GpuWrapper>, GpuError> create()
        {
            return create_gpu_context().transform(
                [](GpuContext ctx) { return std::shared_ptr<GpuWrapper>(new GpuWrapper(std::move(ctx))); });
        }

        // Release resource in m_ctx
        ~GpuWrapper() = default;

        GpuWrapper(const GpuWrapper&) = delete;
        GpuWrapper& operator=(const GpuWrapper&) = delete;

        GpuWrapper(GpuWrapper&&) = delete;
        GpuWrapper& operator=(GpuWrapper&&) = delete;

        [[nodiscard]] auto device() const { return m_ctx.device.raw; }
        [[nodiscard]] SDL_Window *window() const { return m_ctx.window.raw; }

        [[nodiscard]] std::expected<BufferWrapper, GpuError> create_buffer(SDL_GPUBufferUsageFlags flag, uint32_t size);
        [[nodiscard]] std::expected<RenderData, GpuError>
        create_data(const RenderProcedural& render_procedural, uint32_t vertex_count);

        auto release_buffer(SDL_GPUBuffer* buffer)
        {
            if (buffer)
            {
                SDL_ReleaseGPUBuffer(device(), buffer);
            }
        }

        [[nodiscard]] std::expected<RenderProcedural, GpuError> create_render_procedural();

        std::expected<SDL_GPUGraphicsPipeline*, GpuError>
        create_pipeline(const SDL_GPUGraphicsPipelineCreateInfo& create_info)
        {
            auto pipeline = SDL_CreateGPUGraphicsPipeline(device(), &create_info);
            if (!pipeline)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                return std::unexpected(GpuError::CREATE_PIPELINE_FAILED);
            }
            return pipeline;
        }

        auto release_pipeline(SDL_GPUGraphicsPipeline* graphics_pipeline)
        {
            if (graphics_pipeline)
            {
                SDL_ReleaseGPUGraphicsPipeline(device(), graphics_pipeline);
            }
        }

        [[nodiscard]] auto create_shader(const std::vector<uint8_t>& shader, SDL_GPUShaderStage stage,
                                         uint32_t num_uniform_buffers) -> std::expected<SDL_GPUShader*, GpuError>
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
            auto s = SDL_CreateGPUShader(device(), &info);
            if (!s)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                return std::unexpected(GpuError::CREATE_SHADER_FAILED);
            }
            return s;
        }

        auto release_shader(SDL_GPUShader* shader)
        {
            if (shader)
            {
                SDL_ReleaseGPUShader(device(), shader);
            }
        }

        [[nodiscard]] std::expected<SDL_GPUTextureFormat, GpuError> get_texture_format() const
        {
            auto format = SDL_GetGPUSwapchainTextureFormat(device(), window());
            if (format == SDL_GPU_TEXTUREFORMAT_INVALID)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                return std::unexpected(GpuError::GET_TEXTUREFORMAT_FAILED);
            }
            return format;
        }
    };
} // namespace sopho
