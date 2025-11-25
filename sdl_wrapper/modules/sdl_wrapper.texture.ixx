// sdl_wrapper.texture.ixx
// Created by wsqsy on 11/25/2025.
//

module;
#include <SDL3/SDL_gpu.h>
#include <expected>
#include <memory>
export module sdl_wrapper:texture;
import data_type;
import :decl;

namespace sopho
{
    export class TextureWrapper
    {
        std::shared_ptr<GpuWrapper> m_gpu{};
        SDL_GPUTexture* m_texture{};
        SDL_GPUSampler* m_sampler{};
        SDL_GPUTextureSamplerBinding m_tex_binding{};

        TextureWrapper(std::shared_ptr<GpuWrapper> gpu, SDL_GPUTexture* texture, SDL_GPUSampler* sampler) noexcept :
            m_gpu(std::move(gpu)), m_texture(texture), m_sampler(sampler)
        {
            m_tex_binding.texture = m_texture;
            m_tex_binding.sampler = m_sampler;
        }

    public:
        TextureWrapper(const TextureWrapper&) = delete;
        TextureWrapper& operator=(const TextureWrapper&) = delete;
        TextureWrapper(TextureWrapper&& other) noexcept : m_gpu(std::move(other.m_gpu)), m_texture(other.m_texture), m_sampler(other.m_sampler),m_tex_binding(other.m_tex_binding)
        {
            other.m_texture = nullptr;
            other.m_sampler = nullptr;
            other.m_tex_binding = {};
        }

        TextureWrapper& operator=(TextureWrapper&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                m_gpu = std::move(other.m_gpu);
                m_texture = other.m_texture;
                other.m_texture = nullptr;
                m_sampler = other.m_sampler;
                other.m_sampler = nullptr;
                other.m_tex_binding = {};
                m_tex_binding.texture = m_texture;
                m_tex_binding.sampler = m_sampler;
            }
            return *this;
        }

        [[nodiscard]] const SDL_GPUTextureSamplerBinding* get() const noexcept
        {
            return &m_tex_binding;
        }

        void reset() noexcept;

        ~TextureWrapper() noexcept { reset(); }

        struct Builder
        {
            ImageData img_data{};

            Builder& set_image_data(ImageData image_data)
            {
                img_data = image_data;
                return *this;
            }

            std::expected<TextureWrapper, GpuError> build(GpuWrapper& gpu);
        };
    };
} // namespace sopho
