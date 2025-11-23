// data_type.ixx
// Created by sophomore on 11/23/25.
//
module;
#include <expected>
#include <vector>
export module data_type;

export namespace sopho
{
    enum class GpuError
    {
        UNINITIALIZED,
        CREATE_DEVICE_FAILED,
        CREATE_WINDOW_FAILED,
        CLAIM_WINDOW_FAILED,
        CREATE_GPU_BUFFER_FAILED,
        CREATE_TRANSFER_BUFFER_FAILED,
        CREATE_SHADER_FAILED,
        CREATE_PIPELINE_FAILED,
        GET_TEXTUREFORMAT_FAILED,
        BUFFER_OVERFLOW,
        MAP_TRANSFER_BUFFER_FAILED,
        ACQUIRE_COMMAND_BUFFER_FAILED,
        SUBMIT_COMMAND_FAILED,
        BEGIN_COPY_PASS_FAILED,
        COMPILE_VERTEX_SHADER_FAILED,
        COMPILE_FRAGMENT_SHADER_FAILED,
    };

    using TError = GpuError;
    template <typename T>
    using checkable = std::expected<T, TError>;

    struct ImageData
    {
        int width{};
        int height{};
        int channels{};
        std::vector<std::byte> pixels{};
    };

} // namespace sopho
