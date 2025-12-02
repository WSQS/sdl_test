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
        GPUSamplerRaii m_sampler{};
        SDL_GPUTextureSamplerBinding m_tex_binding{};

        TextureWrapper(std::shared_ptr<GpuWrapper> gpu, GpuTextureRaii texture, GPUSamplerRaii sampler) noexcept :
            m_gpu(std::move(gpu)), m_texture(std::move(texture)), m_sampler(std::move(sampler))
        {
            m_tex_binding.texture = m_texture.raw();
            m_tex_binding.sampler = m_sampler.raw();
        }

    public:
        TextureWrapper(const TextureWrapper&) = delete;
        TextureWrapper& operator=(const TextureWrapper&) = delete;
        TextureWrapper(TextureWrapper&& other) noexcept = default;
        TextureWrapper& operator=(TextureWrapper&& other) noexcept = default;
        ~TextureWrapper() noexcept = default;

        [[nodiscard]] const SDL_GPUTextureSamplerBinding* get() const noexcept { return &m_tex_binding; }


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
