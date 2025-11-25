// sdl_wrapper.texture.cpp
// Created by sophomore on 11/25/25.
//
module;
#include <cstdint>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
module sdl_wrapper;
import :texture;
import :transfer_buffer;
import :gpu;
namespace sopho
{

    void TextureWrapper::reset() noexcept
    {
        if (m_texture && m_gpu)
        {
            SDL_ReleaseGPUTexture(m_gpu->device(), m_texture);
            m_texture = nullptr;
        }
        if (m_sampler && m_gpu)
        {
            SDL_ReleaseGPUSampler(m_gpu->device(), m_sampler);
            m_sampler = nullptr;
        }
    }

    std::expected<TextureWrapper, GpuError> TextureWrapper::Builder::build(GpuWrapper& gpu)
    {
        SDL_GPUTextureCreateInfo create_info{.type = SDL_GPU_TEXTURETYPE_2D,
                                             .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                                             .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                                             .width = static_cast<std::uint32_t>(img_data.width),
                                             .height = static_cast<std::uint32_t>(img_data.height),
                                             .layer_count_or_depth = 1,
                                             .num_levels = 1,
                                             .sample_count = SDL_GPU_SAMPLECOUNT_1,
                                             .props = 0};

        auto* texture = SDL_CreateGPUTexture(gpu.device(), &create_info);
        if (!texture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create GPU texture: %s", SDL_GetError());
            return std::unexpected(GpuError::CREATE_TEXTURE_FAILED);
        }

        auto c_tb =
            TransferBufferWrapper::Builder{}
                .set_usage_limit(2)
                .set_size(static_cast<std::uint32_t>(img_data.width) * static_cast<std::uint32_t>(img_data.height) * 4)
                .set_usage(SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD)
                .build(gpu);

        auto submit_result = c_tb.and_then([&](auto tb) { return tb.submit(img_data.pixels.data()); });
        if (!submit_result)
        {
            return std::unexpected{submit_result.error()};
        }
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu.device());
        if (!cmd)
        {
            SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            return std::unexpected{GpuError::ACQUIRE_COMMAND_BUFFER_FAILED};
        }

        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
        if (!copy_pass)
        {
            SDL_Log("SDL_BeginGPUCopyPass failed: %s", SDL_GetError());
            return std::unexpected{GpuError::BEGIN_COPY_PASS_FAILED};
        }

        SDL_GPUTextureTransferInfo src{};
        src.transfer_buffer = c_tb.value().raw();
        src.offset = 0;
        src.pixels_per_row = 0;
        src.rows_per_layer = 0;

        SDL_GPUTextureRegion dst{};
        dst.texture = texture;
        dst.mip_level = 0;
        dst.layer = 0;
        dst.x = 0;
        dst.y = 0;
        dst.z = 0;
        dst.w = static_cast<Uint32>(img_data.width);
        dst.h = static_cast<Uint32>(img_data.height);
        dst.d = 1;

        SDL_UploadToGPUTexture(copy_pass, &src, &dst, false);

        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(cmd);

        SDL_GPUSamplerCreateInfo info{};
        info.min_filter = SDL_GPU_FILTER_LINEAR;
        info.mag_filter = SDL_GPU_FILTER_LINEAR;
        info.max_anisotropy = 1.f;
        info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
        info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
        info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
        info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
        info.enable_anisotropy = true;
        info.enable_compare = false;
        info.compare_op = SDL_GPU_COMPAREOP_ALWAYS;
        info.props = 0;

        auto sampler = SDL_CreateGPUSampler(gpu.device(), &info);
        if (!sampler)
        {
            SDL_Log("SDL_CreateGPUSampler failed: %s", SDL_GetError());
            return std::unexpected{GpuError::CREATE_SAMPLER_FAILED};
        }

        return TextureWrapper{gpu.shared_from_this(), texture, sampler};
    }
} // namespace sopho
