// sdl_wrapper.buffer.ixx
// Created by sophomore on 11/8/25.
//
module;
#include <expected>
#include <memory>
#include <variant>

#include "SDL3/SDL_gpu.h"

export module sdl_wrapper:buffer;
import :decl;

export namespace sopho
{
    class BufferWrapper
    {
        std::shared_ptr<GpuWrapper> m_gpu{}; // Owns the device lifetime
        SDL_GPUBuffer* m_vertex_buffer{}; // Target GPU buffer
        SDL_GPUTransferBuffer* m_transfer_buffer{}; // Staging/transfer buffer
        std::uint32_t m_vertex_buffer_size{}; // Total size of the GPU buffer
        std::uint32_t m_transfer_buffer_size{}; // Current capacity of the transfer buffer

        // Only GpuWrapper is allowed to construct this type.
        BufferWrapper(std::shared_ptr<GpuWrapper> gpu, SDL_GPUBuffer* buffer, std::uint32_t size) noexcept :
            m_gpu(std::move(gpu)), m_vertex_buffer(buffer), m_vertex_buffer_size(size)
        {
        }

    public:
        BufferWrapper(const BufferWrapper&) = delete;
        BufferWrapper& operator=(const BufferWrapper&) = delete;
        BufferWrapper(BufferWrapper&&) = default;
        BufferWrapper& operator=(BufferWrapper&&) = default;

        /// Upload a block of data into the GPU buffer at the given byte offset.
        ///
        /// This function may fail in several ways:
        ///  - The requested upload range exceeds the buffer size.
        ///  - The transfer buffer cannot be created or resized.
        ///  - Mapping the transfer buffer fails.
        ///  - Acquiring a GPU command buffer fails.
        ///
        /// All such failures are reported via the returned std::expected.
        [[nodiscard]] std::expected<std::monostate, GpuError> upload(const void* data, std::uint32_t size,
                                                                     std::uint32_t offset);

        /// Returns the underlying SDL_GPUBuffer pointer.
        [[nodiscard]] SDL_GPUBuffer* data() const noexcept { return m_vertex_buffer; }

        ~BufferWrapper() noexcept;

        friend class GpuWrapper;
    };
} // namespace sopho
