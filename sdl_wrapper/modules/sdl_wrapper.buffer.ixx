// sdl_wrapper.buffer.ixx
// Created by sophomore on 11/8/25.
//
module;
#include <expected>
#include <memory>
#include <variant>
#include <vector>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"

export module sdl_wrapper:buffer;
import data_type;
import :decl;
import :transfer_buffer;

namespace sopho
{
    class BufferWrapper
    {
        std::shared_ptr<GpuWrapper> m_gpu{}; // Owns the device lifetime
        SDL_GPUBuffer* m_gpu_buffer{}; // Target GPU buffer
        TransferBufferWrapper m_transfer_buffer; // Staging/transfer buffer
        std::uint32_t m_buffer_size{}; // Total size of the GPU buffer
        std::vector<std::byte> m_cpu_buffer{};

        // Only GpuWrapper is allowed to construct this type.
        BufferWrapper(std::shared_ptr<GpuWrapper> gpu, SDL_GPUBuffer* gpu_buffer, TransferBufferWrapper transfer_buffer,
                      std::uint32_t size) noexcept :
            m_gpu(std::move(gpu)), m_gpu_buffer(gpu_buffer), m_transfer_buffer(std::move(transfer_buffer)),
            m_buffer_size(size)
        {
            m_cpu_buffer.resize(m_buffer_size);
        }

    public:
        struct Builder
        {
            SDL_GPUBufferUsageFlags flag = SDL_GPU_BUFFERUSAGE_VERTEX;
            std::uint32_t size = 0;

            Builder& set_flag(SDL_GPUBufferUsageFlags usage_flag)
            {
                flag = usage_flag;
                return *this;
            }

            Builder& set_size(std::uint32_t buffer_size)
            {
                size = buffer_size;
                return *this;
            }

            checkable<BufferWrapper> build(GpuWrapper& gpu);
        };

        BufferWrapper(const BufferWrapper&) = delete;
        BufferWrapper& operator=(const BufferWrapper&) = delete;
        BufferWrapper(BufferWrapper&&) = default;
        BufferWrapper& operator=(BufferWrapper&&) = default;

        /// Upload CPU buffer into the GPU buffer at the given byte offset.

        [[nodiscard]] std::expected<std::monostate, GpuError> upload();

        /// Returns the underlying SDL_GPUBuffer pointer.
        [[nodiscard]] SDL_GPUBuffer* gpu_buffer() const noexcept { return m_gpu_buffer; }
        [[nodiscard]] std::byte* cpu_buffer() noexcept { return m_cpu_buffer.data(); }

        ~BufferWrapper() noexcept;

        friend class GpuWrapper;
    };
} // namespace sopho
