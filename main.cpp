#include <array>
#include <iostream>
#include <optional>
#include "shaderc/shaderc.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include "misc/cpp/imgui_stdlib.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "SDL3/SDL_log.h"
import sdl_wrapper;

// the vertex input layout
struct Vertex
{
    float x, y, z; // vec3 position
    float r, g, b, a; // vec4 color
    auto position() { return &x; }
};

class UserApp : public sopho::App
{
    std::shared_ptr<sopho::GpuWrapper> gpu_wrapper{std::make_shared<sopho::GpuWrapper>()};
    sopho::BufferWrapper vertex_buffer{gpu_wrapper->create_buffer(SDL_GPU_BUFFERUSAGE_VERTEX, sizeof(vertices))};
    std::optional<sopho::PipelineWrapper> pipeline_wrapper{std::nullopt};

    SDL_Window* window{};

    // a list of vertices
    std::array<Vertex, 3> vertices{
        Vertex{0.0F, 0.5F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F}, // top vertex
        Vertex{-0.5F, -0.5F, 0.0F, 1.0F, 1.0F, 0.0F, 1.0F}, // bottom left vertex
        Vertex{0.5F, -0.5F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F} // bottom right vertex
    };

    std::string vertex_source =
        R"WSQ(#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 0) out vec4 v_color;

void main()
{
  gl_Position = vec4(a_position, 1.0f);
  v_color = a_color;
})WSQ";
    std::string fragment_source =
        R"WSQ(#version 460

layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = v_color;
})WSQ";

    /**
     * @brief Initialize the application: create the window, configure GPU pipeline and resources, upload initial vertex
     * data, and initialize ImGui.
     *
     * Performs window creation and GPU device claim, configures vertex input and color target state, sets vertex and
     * fragment shaders on the pipeline wrapper and submits pipeline creation, uploads initial vertex data to the vertex
     * buffer, and initializes Dear ImGui (context, style scaling, and SDL3/SDLGPU backends).
     *
     * @return SDL_AppResult `SDL_APP_CONTINUE` to enter the main loop, `SDL_APP_SUCCESS` to request immediate
     * termination.
     */
    virtual SDL_AppResult init(int argc, char** argv) override
    {
        // create a window
        window = SDL_CreateWindow("Hello, Triangle!", 960, 540, SDL_WINDOW_RESIZABLE);
        gpu_wrapper->set_window(window);

        SDL_ClaimWindowForGPUDevice(gpu_wrapper->data(), window);

        pipeline_wrapper.emplace(gpu_wrapper);

        pipeline_wrapper->set_vertex_shader(vertex_source);
        pipeline_wrapper->set_fragment_shader(fragment_source);
        pipeline_wrapper->submit();

        vertex_buffer.upload(&vertices, sizeof(vertices), 0);


        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        // Bake a fixed style scale. (until we have a solution for dynamic style
        // scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;
        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this
        // unnecessary. We leave both here for documentation purpose)

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLGPU(window);
        ImGui_ImplSDLGPU3_InitInfo init_info = {};
        init_info.Device = gpu_wrapper->data();
        init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_wrapper->data(), window);
        init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1; // Only used in multi-viewports mode.
        init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR; // Only used in multi-viewports mode.
        init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
        ImGui_ImplSDLGPU3_Init(&init_info);

        return SDL_APP_CONTINUE;
    }

    /**
     * @brief Advance the application by one frame: update UI, apply vertex edits and live shader recompilation, render,
     * and submit GPU work.
     *
     * Processes ImGui frames, uploads vertex data when edited, recompiles and replaces the vertex shader and graphics
     * pipeline on shader edits, records a render pass that draws the triangle and ImGui draw lists, and submits the GPU
     * command buffer for presentation.
     *
     * @return `SDL_APP_CONTINUE` to continue the main loop.
     */
    virtual SDL_AppResult iterate() override
    {
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        {
            ImGui::Begin("NodeEditor");
            auto change = ImGui::DragFloat3("node1", vertices[0].position(), 0.01f, -1.f, 1.f);
            change = ImGui::DragFloat3("node2", vertices[1].position(), 0.01f, -1.f, 1.f) || change;
            change = ImGui::DragFloat3("node3", vertices[2].position(), 0.01f, -1.f, 1.f) || change;
            if (change)
            {
                vertex_buffer.upload(&vertices, sizeof(vertices), 0);
            }
            ImGui::End();
        }
        {
            ImGui::Begin("SourceEditor");
            auto line_count = std::count(vertex_source.begin(), vertex_source.end(), '\n');
            ImVec2 size =
                ImVec2(ImGui::GetContentRegionAvail().x,
                       std::min(ImGui::GetTextLineHeight() * (line_count + 3), ImGui::GetContentRegionAvail().y));
            if (ImGui::InputTextMultiline("code editor", &vertex_source, size, ImGuiInputTextFlags_AllowTabInput))
            {
                pipeline_wrapper->set_vertex_shader(vertex_source);
            }

            ImGui::End();
        }

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        pipeline_wrapper->submit();

        // acquire the command buffer
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(gpu_wrapper->data());

        // get the swapchain texture
        SDL_GPUTexture* swapchainTexture;
        Uint32 width, height;
        SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, &width, &height);

        // end the frame early if a swapchain texture is not available
        if (swapchainTexture == NULL)
        {
            // you must always submit the command buffer
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return SDL_APP_CONTINUE;
        }

        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, commandBuffer);

        // create the color target
        SDL_GPUColorTargetInfo colorTargetInfo{};
        colorTargetInfo.clear_color = {240 / 255.0F, 240 / 255.0F, 240 / 255.0F, 255 / 255.0F};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = swapchainTexture;

        // begin a render pass
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

        // draw calls go here
        SDL_BindGPUGraphicsPipeline(renderPass, pipeline_wrapper->data());

        // bind the vertex buffer
        SDL_GPUBufferBinding bufferBindings[1];
        bufferBindings[0].buffer = vertex_buffer.data(); // index 0 is slot 0 in this example
        bufferBindings[0].offset = 0; // start from the first byte

        SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1); // bind one buffer starting from slot 0

        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, commandBuffer, renderPass);
        // end the render pass
        SDL_EndGPURenderPass(renderPass);

        // submit the command buffer
        SDL_SubmitGPUCommandBuffer(commandBuffer);

        return SDL_APP_CONTINUE;
    }

    /**
     * @brief Handle an SDL event by forwarding it to ImGui and handling window-close requests.
     *
     * @param event Pointer to the SDL event to process.
     * @return SDL_AppResult `SDL_APP_SUCCESS` when a window close was requested, `SDL_APP_CONTINUE` otherwise.
     */
    virtual SDL_AppResult event(SDL_Event* event) override
    {
        ImGui_ImplSDL3_ProcessEvent(event);
        // close the window on request
        if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            return SDL_APP_SUCCESS;
        }

        return SDL_APP_CONTINUE;
    }

    /**
     * @brief Clean up UI and GPU resources and close the application window.
     *
     * Shuts down ImGui SDL3 and SDLGPU backends, destroys the ImGui context,
     * releases the application's association with the GPU device for the window,
     * and destroys the SDL window.
     *
     * @param result Application exit result code provided by the SDL app framework.
     */
    virtual void quit(SDL_AppResult result) override
    {
        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
        ImGui::DestroyContext();

        SDL_ReleaseWindowFromGPUDevice(gpu_wrapper->data(), window);

        // destroy the window
        SDL_DestroyWindow(window);
    }
};

/**
 * @brief Creates a new application instance for the program.
 *
 * @param argc Program argument count as passed to main.
 * @param argv Program argument vector as passed to main.
 * @return sopho::App* Pointer to a heap-allocated application object; the caller takes ownership and is responsible for
 * deleting it.
 */
sopho::App* create_app(int argc, char** argv) { return new UserApp(); }
