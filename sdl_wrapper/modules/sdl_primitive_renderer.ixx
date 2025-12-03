// sdl_primitive_renderer.ixx
// Created by wsqsy on 12/3/2025.
//
module;
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <cassert>
#include <expected>
#include <map>
#include <memory>
#include <variant>
#include <vector>
export module sdl_primitive_renderer;
import primitive_renderer;
import data_type;
import sdl_raii;
import sdl_wrapper;
namespace sopho
{
    export class SDLPrimitiveRenderer : public PrimitiveRenderer
    {
        std::shared_ptr<GpuWrapper> m_gpu{};
        SDLPrimitiveRenderer(std::shared_ptr<GpuWrapper> gpu) : m_gpu(std::move(gpu)) {}
        GpuCommandBufferRaii m_gpu_command_buffer{};
        GpuRenderPassRaii m_gpu_render_pass{};
        std::map<RenderProcedureHandle, std::shared_ptr<RenderProcedural>> m_render_procedures{};
        std::map<TextureHandle, GpuTextureRaii> m_textures{};
        std::map<BufferHandle, GpuBufferRaii> m_buffers{};
        SDL_GPUTexture* m_swapchain_texture{};
        GpuTextureRaii m_depth_texture{};

    public:
        static checkable<PrimitiveRenderer*> create()
        {
            auto gpu_result = GpuWrapper::create();
            if (!gpu_result)
            {
                return std::unexpected(gpu_result.error());
            }
            return new SDLPrimitiveRenderer{gpu_result.value()};
        }
        ~SDLPrimitiveRenderer() override = default;

        void begin_frame() override
        {
            auto p_buffer = SDL_AcquireGPUCommandBuffer(m_gpu->device());
            m_gpu_command_buffer.reset(p_buffer);
            std::uint32_t width = 0, height = 0;
            SDL_WaitAndAcquireGPUSwapchainTexture(m_gpu_command_buffer.raw(), m_gpu->window(), &m_swapchain_texture,
                                                  &width, &height);
            SDL_GPUTextureCreateInfo ci = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
                .width = static_cast<std::uint32_t>(width),
                .height = static_cast<std::uint32_t>(height),
                .layer_count_or_depth = 1,
                .num_levels = 1,
                .sample_count = SDL_GPU_SAMPLECOUNT_1,
            };
            auto SceneDepthTexture = SDL_CreateGPUTexture(m_gpu->device(), &ci);
            m_depth_texture.reset(m_gpu->device(), SceneDepthTexture);
        }
        void end_frame() override { m_gpu_command_buffer.reset(); }
        void begin_render_pass(const RenderPassDescriptor& render_pass_descriptor) override
        {
            if (!m_swapchain_texture)
            {
                return;
            }
            SDL_GPUColorTargetInfo colorTargetInfo{};
            colorTargetInfo.clear_color = {135 / 255.0F, 135 / 255.0F, 135 / 255.0F, 255 / 255.0F};
            if (render_pass_descriptor.clear)
            {
                colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            }
            else
            {
                colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
            }
            colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
            colorTargetInfo.texture = m_swapchain_texture;

            SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{};
            depthStencilTargetInfo.texture = m_depth_texture.raw();
            depthStencilTargetInfo.cycle = true;
            depthStencilTargetInfo.clear_depth = 1;
            depthStencilTargetInfo.clear_stencil = 0;
            depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
            depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
            depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
            if (render_pass_descriptor.depth)
            {
                auto p_render_pass =
                    SDL_BeginGPURenderPass(m_gpu_command_buffer.raw(), &colorTargetInfo, 1, &depthStencilTargetInfo);
                m_gpu_render_pass.reset(p_render_pass);
            }
            else
            {
                auto p_render_pass = SDL_BeginGPURenderPass(m_gpu_command_buffer.raw(), &colorTargetInfo, 1, nullptr);
                m_gpu_render_pass.reset(p_render_pass);
            }
        }
        void end_render_pass() override { m_gpu_render_pass.reset(); }
        checkable<RenderProcedureHandle>
        create_render_procedure(const RenderProcedureDescriptor& render_procedure_descriptor) override
        {
            auto pw_result = m_gpu->create_render_procedural();
            if (!pw_result)
            {
                return std::unexpected(pw_result.error());
            }

            auto pipeline_init =
                pw_result
                    .and_then([&](auto& pipeline)
                              { return pipeline.set_vertex_shader(render_procedure_descriptor.vert_shader); })
                    .and_then([&](std::monostate)
                              { return pw_result->set_fragment_shader(render_procedure_descriptor.frag_shader); })
                    .and_then([&](std::monostate) { return pw_result->submit(); });

            if (!pipeline_init)
            {
                return std::unexpected(pipeline_init.error());
            }
            RenderProcedureHandle handle{};
            if (!m_render_procedures.empty())
            {
                handle = static_cast<RenderProcedureHandle>(
                    static_cast<std::uint32_t>(m_render_procedures.rbegin()->first) + 1);
            }
            m_render_procedures[handle] = std::make_shared<RenderProcedural>(std::move(pw_result.value()));
            return handle;
        }
        void bind_render_procedure(const RenderProcedureHandle& render_procedure_handle) override
        {
            if (m_render_procedures.contains(render_procedure_handle))
            {
                SDL_BindGPUGraphicsPipeline(m_gpu_render_pass.raw(),
                                            m_render_procedures[render_procedure_handle]->raw());
            }
        }
        checkable<BufferHandle> create_buffer(const BufferDescriptor& buffer_descriptor) override
        {
            SDL_GPUBufferUsageFlags usage{};
            switch (buffer_descriptor.buffer_usage)
            {
            case BufferUsage::VERTEX:
                usage = SDL_GPU_BUFFERUSAGE_VERTEX;
                break;
            case BufferUsage::INDEX:
                usage = SDL_GPU_BUFFERUSAGE_INDEX;
                break;
            default:
                assert(!"Invalid buffer usage");
                break;
            }
            SDL_GPUBufferCreateInfo create_info{.usage = usage,
                                                .size = static_cast<std::uint32_t>(buffer_descriptor.data.size())};
            auto gpu_buffer = SDL_CreateGPUBuffer(m_gpu->device(), &create_info);
            if (!gpu_buffer)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                return std::unexpected(GpuError::CREATE_GPU_BUFFER_FAILED);
            }
            GpuBufferRaii gpu_buffer_raii{m_gpu->device(), gpu_buffer};
            SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info{};
            transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            transfer_buffer_create_info.size = buffer_descriptor.data.size();

            auto* transfer_buffer = SDL_CreateGPUTransferBuffer(m_gpu->device(), &transfer_buffer_create_info);
            if (!transfer_buffer)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d %s", __FILE__, __LINE__, SDL_GetError());
                return std::unexpected(GpuError::CREATE_TRANSFER_BUFFER_FAILED);
            }
            GpuTransferBufferRaii tb_raii{m_gpu->device(), transfer_buffer};

            void* dst = SDL_MapGPUTransferBuffer(m_gpu->device(), tb_raii.raw(), false);
            if (!dst)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to map transfer buffer: %s", __FILE__, __LINE__,
                             SDL_GetError());

                return std::unexpected(GpuError::MAP_TRANSFER_BUFFER_FAILED);
            }

            SDL_memcpy(dst, buffer_descriptor.data.data(), buffer_descriptor.data.size());
            SDL_UnmapGPUTransferBuffer(m_gpu->device(), tb_raii.raw());
            GpuCommandBufferRaii command_buffer_raii;
            {
                // 3. Acquire a command buffer and enqueue the copy pass.
                auto* command_buffer = SDL_AcquireGPUCommandBuffer(m_gpu->device());
                if (!command_buffer)
                {
                    SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to acquire GPU command buffer: %s", __FILE__,
                                 __LINE__, SDL_GetError());

                    return std::unexpected(GpuError::ACQUIRE_COMMAND_BUFFER_FAILED);
                }
                command_buffer_raii.reset(command_buffer);
            }

            auto* copy_pass = SDL_BeginGPUCopyPass(command_buffer_raii.raw());

            if (!copy_pass)
            {
                SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s:%d failed to begin GPU copy pass: %s", __FILE__, __LINE__,
                             SDL_GetError());
                return std::unexpected(GpuError::BEGIN_COPY_PASS_FAILED);
            }

            SDL_GPUTransferBufferLocation location{};
            location.transfer_buffer = tb_raii.raw();
            location.offset = 0;

            SDL_GPUBufferRegion region{};
            region.buffer = gpu_buffer_raii.raw();
            region.size = buffer_descriptor.data.size();
            region.offset = 0;

            SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);

            SDL_EndGPUCopyPass(copy_pass);
            BufferHandle handle{};
            if (!m_buffers.empty())
            {
                handle = static_cast<BufferHandle>(static_cast<std::uint32_t>(m_buffers.rbegin()->first) + 1);
            }
            m_buffers[handle] = std::move(gpu_buffer_raii);
            return handle;
        }
        void bind_vertex_buffer(const BufferHandle& buffer_handle) override
        {
            std::vector<SDL_GPUBufferBinding> bindings;
            bindings.emplace_back(SDL_GPUBufferBinding{m_buffers[buffer_handle].raw(), 0});
            SDL_BindGPUVertexBuffers(m_gpu_render_pass.raw(), 0, bindings.data(), bindings.size());
        }
        void bind_index_buffer(const BufferHandle& buffer_handle) override
        {
            std::vector<SDL_GPUBufferBinding> bindings;
            bindings.emplace_back(SDL_GPUBufferBinding{m_buffers[buffer_handle].raw(), 0});
            SDL_BindGPUIndexBuffer(m_gpu_render_pass.raw(), bindings.data(), SDL_GPU_INDEXELEMENTSIZE_32BIT);
        }
        void push_vertex_uniform(const UniformDescriptor& uniform_descriptor) override
        {
            SDL_PushGPUVertexUniformData(m_gpu_command_buffer.raw(), uniform_descriptor.slot_index,
                                         uniform_descriptor.data.data(), uniform_descriptor.data.size());
        }
        void push_fragment_uniform(const UniformDescriptor& uniform_descriptor) override
        {
            SDL_PushGPUFragmentUniformData( m_gpu_command_buffer.raw(), uniform_descriptor.slot_index,
                                         uniform_descriptor.data.data(), uniform_descriptor.data.size());
        }
        auto& get_gpu() { return *m_gpu; }
        auto& get_command_buffer() { return m_gpu_command_buffer; }
        auto& get_render_pass() { return m_gpu_render_pass; }
    };
} // namespace sopho
