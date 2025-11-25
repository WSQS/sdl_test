// sdl_wrapper.texture.ixx
// Created by wsqsy on 11/25/2025.
//

module;
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <expected>
#include <memory>
export module sdl_wrapper:texture;
import data_type;
import :decl;
import :gpu;

namespace sopho
{
    class TextureWrapper
    {
        std::shared_ptr<GpuWrapper> m_gpu{};
        SDL_GPUTexture* m_texture{};

        TextureWrapper(std::shared_ptr<GpuWrapper> gpu, SDL_GPUTexture* texture) noexcept :
            m_gpu(std::move(gpu)), m_texture(texture)
        {
        }

    public:
        TextureWrapper(const TextureWrapper&) = delete;
        TextureWrapper& operator=(const TextureWrapper&) = delete;
        TextureWrapper(TextureWrapper&& other) noexcept : m_gpu(std::move(other.m_gpu)), m_texture(other.m_texture)
        {
            other.m_texture = nullptr;
        }

        TextureWrapper& operator=(TextureWrapper&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                m_gpu = std::move(other.m_gpu);
                m_texture = other.m_texture;
                other.m_texture = nullptr;
            }
            return *this;
        }

        [[nodiscard]] SDL_GPUTexture* get() const noexcept { return m_texture; }

        void reset() noexcept
        {
            if (m_texture && m_gpu)
            {
                SDL_ReleaseGPUTexture(m_gpu->device(), m_texture);
                m_texture = nullptr;
            }
        }

        ~TextureWrapper() noexcept { reset(); }

        struct Builder
        {
            SDL_GPUTextureType type = SDL_GPU_TEXTURETYPE_2D;
            SDL_GPUTextureFormat format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
            SDL_GPUTextureUsageFlags usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
            std::uint32_t width = 1;
            std::uint32_t height = 1;
            std::uint32_t layer_count_or_depth = 1;
            std::uint32_t num_levels = 1;
            SDL_GPUSampleCount sample_count = SDL_GPU_SAMPLECOUNT_1;

            Builder& set_type(SDL_GPUTextureType texture_type)
            {
                type = texture_type;
                return *this;
            }

            Builder& set_format(SDL_GPUTextureFormat texture_format)
            {
                format = texture_format;
                return *this;
            }

            Builder& set_usage(SDL_GPUTextureUsageFlags texture_usage)
            {
                usage = texture_usage;
                return *this;
            }

            Builder& set_width(std::uint32_t tex_width)
            {
                width = tex_width;
                return *this;
            }

            Builder& set_height(std::uint32_t tex_height)
            {
                height = tex_height;
                return *this;
            }

            Builder& set_layer_count_or_depth(std::uint32_t count_or_depth)
            {
                layer_count_or_depth = count_or_depth;
                return *this;
            }

            Builder& set_num_levels(std::uint32_t levels)
            {
                num_levels = levels;
                return *this;
            }

            Builder& set_sample_count(SDL_GPUSampleCount samples)
            {
                sample_count = samples;
                return *this;
            }

            std::expected<TextureWrapper, GpuError> build(GpuWrapper& gpu)
            {
                SDL_GPUTextureCreateInfo create_info{.type = type,
                                                     .format = format,
                                                     .usage = usage,
                                                     .width = width,
                                                     .height = height,
                                                     .layer_count_or_depth = layer_count_or_depth,
                                                     .num_levels = num_levels,
                                                     .sample_count = sample_count,
                                                     .props = 0};

                auto* texture = SDL_CreateGPUTexture(gpu.device(), &create_info);
                if (!texture)
                {
                    SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create GPU texture: %s", SDL_GetError());
                    return std::unexpected(GpuError::CREATE_GPU_TEXTURE_FAILED);
                }

                return TextureWrapper{gpu.shared_from_this(), texture};
            }
        };
    };
} // namespace sopho
