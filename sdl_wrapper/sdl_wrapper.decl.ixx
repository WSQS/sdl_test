//
// Created by sophomore on 11/12/25.
//

export module sdl_wrapper:decl;

export namespace sopho
{
    class App;
    class GpuWrapper;
    class BufferWrapper;
    class PipelineWrapper;
    
    /**
     * @brief Create an application instance with command line arguments.
     * 
     * @param argc The number of command line arguments (including program name).
     * @param argv Array of command line argument strings.
     * @return sopho::App* Pointer to the created application instance, or nullptr on failure.
     */
    extern sopho::App* create_app(int argc, char** argv);
}
