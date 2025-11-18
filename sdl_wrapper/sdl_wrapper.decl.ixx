//
// Created by sophomore on 11/12/25.
//

export module sdl_wrapper:decl;

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

    class App;
    class GpuWrapper;
    class BufferWrapper;
    class RenderProcedural;
}
