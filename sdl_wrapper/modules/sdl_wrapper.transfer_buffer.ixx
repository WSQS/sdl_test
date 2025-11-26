// sdl_wrapper.transfer_buffer.ixx
// Created by sophomore on 11/24/25.
//
/*
 * @file sdl_wrapper.transfer_buffer.ixx
 * @brief Transfer buffer wrapper module for SDL GPU operations.
 *
 * This module provides a wrapper for SDL GPU transfer buffers with a builder pattern
 * that allows specifying usage limits for automatic resource management.
 */
module;
#include <cassert>
#include <cstdint>
#include <expected>
#include <memory>
#include <variant>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
export module sdl_wrapper:transfer_buffer;
import data_type;
import :decl;
namespace sopho
{

    /*
     * @brief Wrapper class for SDL GPU transfer buffer operations.
     */
    class TransferBufferWrapper
    {
    private:
        std::shared_ptr<GpuWrapper> m_gpu{}; // Owns the device lifetime
        SDL_GPUTransferBuffer* m_transfer_buffer{}; // The actual transfer buffer
        std::int32_t m_usage_limit{}; // Current usage count
        std::uint32_t m_size{}; // Size of the transfer buffer in bytes

        // Private constructor to ensure only Builder can create instances
        TransferBufferWrapper(std::shared_ptr<GpuWrapper> gpu, SDL_GPUTransferBuffer* transfer_buffer,
                              std::int32_t usage_limit, std::uint32_t size) noexcept :
            m_gpu(std::move(gpu)), m_transfer_buffer(transfer_buffer), m_usage_limit(usage_limit), m_size(size)
        {
        }

    public:
        TransferBufferWrapper(const TransferBufferWrapper&) = delete;
        TransferBufferWrapper& operator=(const TransferBufferWrapper&) = delete;
        TransferBufferWrapper(TransferBufferWrapper&& other) noexcept :
            m_gpu(std::move(other.m_gpu)), m_transfer_buffer(other.m_transfer_buffer),
            m_usage_limit(other.m_usage_limit), m_size(other.m_size)
        {
            other.m_transfer_buffer = nullptr;
        }

        TransferBufferWrapper& operator=(TransferBufferWrapper&& other) noexcept;

        /*
         * @brief Destructor that releases the transfer buffer.
         */
        ~TransferBufferWrapper() noexcept { reset(); }

        /*
         * @brief Releases the underlying transfer buffer and resets state.
         */
        void reset() noexcept;

        /*
         * @brief Returns the underlying SDL_GPUTransferBuffer pointer.
         */
        [[nodiscard]] SDL_GPUTransferBuffer* raw() const noexcept { return m_transfer_buffer; }

        /*
         * @brief Returns the size of the transfer buffer in bytes.
         */
        [[nodiscard]] std::uint32_t size() const noexcept { return m_size; }

        checkable<std::monostate> submit(const void* data_source);

        /*
         * @brief Builder pattern for creating TransferBufferWrapper instances.
         *
         * Provides a fluent interface for configuring transfer buffer properties
         * such as usage limit, buffer type, and size.
         */
        struct Builder
        {
            /*
             * @brief Maximum number of uploads allowed before the buffer is recycled.
             * When this limit is reached, the buffer will be automatically recycled.
             */
            std::int32_t usage_limit{1};

            /* @brief The usage type for the transfer buffer (e.g., upload/download). */
            SDL_GPUTransferBufferUsage usage{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD};

            /* @brief Size of the transfer buffer in bytes. */
            std::uint32_t size{1024};

            /*
             * @brief Sets the usage limit for the transfer buffer.
             * @param limit Maximum number of uploads before the buffer is recycled
             * @return Reference to this Builder for chaining
             */
            Builder& set_usage_limit(std::int32_t limit)
            {
                usage_limit = limit;
                return *this;
            }

            /*
             * @brief Sets the usage type for the transfer buffer.
             * @param buffer_usage The SDL GPU transfer buffer usage type
             * @return Reference to this Builder for chaining
             */
            Builder& set_usage(SDL_GPUTransferBufferUsage buffer_usage)
            {
                usage = buffer_usage;
                return *this;
            }

            /*
             * @brief Sets the size of the transfer buffer.
             * @param buffer_size Size in bytes
             * @return Reference to this Builder for chaining
             */
            Builder& set_size(std::uint32_t buffer_size)
            {
                size = buffer_size;
                return *this;
            }

            /*
             * @brief Builds the TransferBufferWrapper instance.
             * @param gpu The GPU wrapper to use for creating the transfer buffer
             * @return A checkable containing the TransferBufferWrapper on success, or an error on failure
             */
            checkable<TransferBufferWrapper> build(GpuWrapper& gpu);
        };
    };
} // namespace sopho
