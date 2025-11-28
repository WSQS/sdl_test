// sdl_wrapper.texture.ixx
// Created by wsqsy on 11/25/2025.
//

module;
#include <SDL3/SDL_gpu.h>
#include <expected>
#include <memory>
#include <utility>
export module sdl_wrapper:texture;
import data_type;
import sdl_raii;
import :decl;

namespace sopho
{
    export class TextureWrapper
    {
        std::shared_ptr<GpuWrapper> m_gpu{};
        GpuTextureRaii m_texture{};
        SDL_GPUSampler* m_sampler{};
        SDL_GPUTextureSamplerBinding m_tex_binding{};

        TextureWrapper(std::shared_ptr<GpuWrapper> gpu, GpuTextureRaii texture, SDL_GPUSampler* sampler) noexcept :
            m_gpu(std::move(gpu)), m_texture(std::move(texture)), m_sampler(sampler)
        {
            m_tex_binding.texture = m_texture.raw();
            m_tex_binding.sampler = m_sampler;
        }

    public:
        TextureWrapper(const TextureWrapper&) = delete;
        TextureWrapper& operator=(const TextureWrapper&) = delete;
        TextureWrapper(TextureWrapper&& other) noexcept : m_gpu(std::move(other.m_gpu)), m_texture(std::move(other.m_texture)), m_sampler(other.m_sampler),m_tex_binding(other.m_tex_binding)
        {
            other.m_sampler = nullptr;
            other.m_tex_binding = {};
        }

        TextureWrapper& operator=(TextureWrapper&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                m_gpu = std::move(other.m_gpu);
                m_texture = std::move(other.m_texture);
                m_sampler = other.m_sampler;
                other.m_sampler = nullptr;
                other.m_tex_binding = {};
                m_tex_binding.texture = m_texture.raw();
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
                img_data = std::move(image_data);
                return *this;
            }

            std::expected<TextureWrapper, GpuError> build(GpuWrapper& gpu);
        };
    };
} // namespace sopho
