//
// Created by sophomore on 11/12/25.
//

export module sdl_wrapper:decl;

export namespace sopho
{
    enum class GpuError
    {
        CREATE_DEVICE_FAILED,
        CREATE_WINDOW_FAILED,
        CREATE_BUFFER_FAILED,
        CREATE_SHADER_FAILED,
        CREATE_PIPELINE_FAILED,
        GET_TEXTUREFORMAT_FAILED,
    };

    class App;
    class GpuWrapper;
    class BufferWrapper;
    class PipelineWrapper;
}
