#include <array>
#include <iostream>
#include <optional>
#include "shaderc/shaderc.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include "misc/cpp/imgui_stdlib.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_gpu.h>

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
    std::optional<sopho::PipelineWrapper> pipeline_wrapper;
    std::shared_ptr<sopho::GpuWrapper> gpu_wrapper{std::make_shared<sopho::GpuWrapper>()};
    sopho::BufferWrapper vertex_buffer{gpu_wrapper->create_buffer(SDL_GPU_BUFFERUSAGE_VERTEX, sizeof(vertices))};

    SDL_Window* window{};
    SDL_GPUGraphicsPipeline* graphicsPipeline{};

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

    shaderc::Compiler compiler{};
    shaderc::CompileOptions options{};

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
    SDL_GPUShader* vertexShader{};
    SDL_GPUShader* fragmentShader{};

    SDL_GPUColorTargetDescription colorTargetDescriptions[1]{};
    SDL_GPUVertexAttribute vertexAttributes[2]{};
    SDL_GPUVertexBufferDescription vertexBufferDesctiptions[1]{};

    /**
     * @brief Get the SDL_GPUDevice from gpu_wrapper with validation.
     *
     * Logs an error and returns nullptr if the device is not available.
     */
    SDL_GPUDevice* get_device_or_log() const noexcept
    {
        if (!gpu_wrapper)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "GpuWrapper is null: GPU device is not available");
            return nullptr;
        }

        SDL_GPUDevice* device = gpu_wrapper->data();
        if (device == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "GPU device creation failed (gpu_wrapper->data() returned null)");
        }
        return device;
    }

    /**
     * @brief Initialize the application by creating the window, compiling shaders, creating the GPU pipeline and
     * resources, and initializing ImGui.
     *
     * Performs window creation and GPU device claim, compiles vertex and fragment GLSL to SPIR-V and creates GPU shader
     * objects, configures and creates the graphics pipeline (vertex input, attributes, and color target/blend state),
     * uploads initial vertex data to the vertex buffer, and initializes Dear ImGui (context, style, scaling, and
     * SDL3/SDLGPU backends).
     *
     * @return SDL_AppResult `SDL_APP_CONTINUE` to enter the main loop, `SDL_APP_SUCCESS` to request immediate
     * termination.
     */
    virtual SDL_AppResult init(int argc, char** argv) override
    {
        (void)argc;
        (void)argv;

        // create a window
        window = SDL_CreateWindow("Hello, Triangle!", 960, 540, SDL_WINDOW_RESIZABLE);
        if (!window)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Failed to create SDL window: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        SDL_GPUDevice* device = get_device_or_log();
        if (device == nullptr)
        {
            // GPU device creation failed; cannot proceed.
            return SDL_APP_FAILURE;
        }

        if (SDL_ClaimWindowForGPUDevice(device, window) == SDL_FALSE)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Failed to claim window for GPU device: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
        auto result = compiler.CompileGlslToSpv(vertex_source, shaderc_glsl_vertex_shader, "test.glsl", options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                         "[shaderc] compile error in test.glsl: %s",
                         result.GetErrorMessage().c_str());
            return SDL_APP_FAILURE;
        }

        // load the vertex shader code
        std::vector<uint32_t> vertexCode{result.cbegin(), result.cend()};

        // create the vertex shader
        SDL_GPUShaderCreateInfo vertexInfo{};
        vertexInfo.code = reinterpret_cast<Uint8*>(vertexCode.data());
        vertexInfo.code_size = vertexCode.size() * sizeof(uint32_t);
        vertexInfo.entrypoint = "main";
        vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        vertexInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
        vertexInfo.num_samplers = 0;
        vertexInfo.num_storage_buffers = 0;
        vertexInfo.num_storage_textures = 0;
        vertexInfo.num_uniform_buffers = 0;

        vertexShader = SDL_CreateGPUShader(device, &vertexInfo);
        if (!vertexShader)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                         "Failed to create vertex shader: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        result = compiler.CompileGlslToSpv(fragment_source, shaderc_glsl_fragment_shader, "test.frag", options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                         "[shaderc] compile error in test.frag: %s",
                         result.GetErrorMessage().c_str());
            return SDL_APP_FAILURE;
        }

        // load the fragment shader code
        std::vector<uint32_t> fragmentCode{result.cbegin(), result.cend()};

        // create the fragment shader
        SDL_GPUShaderCreateInfo fragmentInfo{};
        fragmentInfo.code = reinterpret_cast<Uint8*>(fragmentCode.data());
        fragmentInfo.code_size = fragmentCode.size() * sizeof(uint32_t);
        fragmentInfo.entrypoint = "main";
        fragmentInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        fragmentInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        fragmentInfo.num_samplers = 0;
        fragmentInfo.num_storage_buffers = 0;
        fragmentInfo.num_storage_textures = 0;
        fragmentInfo.num_uniform_buffers = 0;

        fragmentShader = SDL_CreateGPUShader(device, &fragmentInfo);
        if (!fragmentShader)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                         "Failed to create fragment shader: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // create the graphics pipeline
        pipelineInfo.vertex_shader = vertexShader;
        pipelineInfo.fragment_shader = fragmentShader;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

        // describe the vertex buffers
        vertexBufferDesctiptions[0].slot = 0;
        vertexBufferDesctiptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vertexBufferDesctiptions[0].instance_step_rate = 0;
        vertexBufferDesctiptions[0].pitch = sizeof(Vertex);

        pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesctiptions;

        // describe the vertex attribute
        // a_position
        vertexAttributes[0].buffer_slot = 0;
        vertexAttributes[0].location = 0;
        vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        vertexAttributes[0].offset = 0;

        // a_color
        vertexAttributes[1].buffer_slot = 0;
        vertexAttributes[1].location = 1;
        vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        vertexAttributes[1].offset = sizeof(float) * 3;

        pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
        pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

        // describe the color target
        colorTargetDescriptions[0] = {};
        colorTargetDescriptions[0].blend_state.enable_blend = true;
        colorTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        colorTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        colorTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(device, window);

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

        // create the pipeline
        graphicsPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
        if (!graphicsPipeline)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                         "Failed to create graphics pipeline: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // upload initial vertex data
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

        float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLGPU(window);
        ImGui_ImplSDLGPU3_InitInfo init_info = {};
        init_info.Device = device;
        init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
        init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1; // Only used in multi-viewports mode.
        init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR; // Only used in multi-viewports mode.
        init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
        ImGui_ImplSDLGPU3_Init(&init_info);

        return SDL_APP_CONTINUE;
    }

    /**
     * @brief Advance the application by one frame: update UI state, handle vertex edits and live shader recompilation,
     * and render.
     *
     * Processes ImGui frames, applies interactive vertex edits (uploading vertex data when changed), recompiles and
     * replaces the vertex shader and graphics pipeline on edits, acquires a GPU command buffer and swapchain texture,
     * records a render pass that draws the triangle and ImGui draw lists, and submits the command buffer.
     *
     * @return SDL_AppResult SDL_APP_CONTINUE to continue the main loop.
     */
    virtual SDL_AppResult iterate() override
    {
        SDL_GPUDevice* device = get_device_or_log();
        if (!device)
        {
            // Fatal GPU state; request exit.
            return SDL_APP_FAILURE;
        }

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
                auto result =
                    compiler.CompileGlslToSpv(vertex_source, shaderc_glsl_vertex_shader, "test.glsl", options);

                if (result.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    std::cerr << "[shaderc] compile error in "
                              << "test.glsl"
                              << ":\n"
                              << result.GetErrorMessage() << std::endl;
                    SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                                 "[shaderc] compile error in test.glsl: %s",
                                 result.GetErrorMessage().c_str());
                }
                else
                {
                    // load the vertex shader code
                    std::vector<uint32_t> vertexCode{result.cbegin(), result.cend()};

                    // create the vertex shader
                    SDL_GPUShaderCreateInfo vertexInfo{};
                    vertexInfo.code = reinterpret_cast<Uint8*>(vertexCode.data());
                    vertexInfo.code_size = vertexCode.size() * sizeof(uint32_t);
                    vertexInfo.entrypoint = "main";
                    vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
                    vertexInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
                    vertexInfo.num_samplers = 0;
                    vertexInfo.num_storage_buffers = 0;
                    vertexInfo.num_storage_textures = 0;
                    vertexInfo.num_uniform_buffers = 0;

                    SDL_ReleaseGPUShader(device, vertexShader);
                    vertexShader = SDL_CreateGPUShader(device, &vertexInfo);
                    if (!vertexShader)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                                     "Failed to recreate vertex shader: %s", SDL_GetError());
                    }
                    else
                    {
                        pipelineInfo.vertex_shader = vertexShader;
                        SDL_ReleaseGPUGraphicsPipeline(device, graphicsPipeline);
                        graphicsPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
                        if (!graphicsPipeline)
                        {
                            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                                         "Failed to recreate graphics pipeline: %s", SDL_GetError());
                        }
                    }
                }
            }

            ImGui::End();
        }

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        // acquire the command buffer
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
        if (!commandBuffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU,
                         "Failed to acquire GPU command buffer: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // get the swapchain texture
        SDL_GPUTexture* swapchainTexture;
        Uint32 width, height;
        if (SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, &width, &height) ==
                SDL_FALSE ||
            swapchainTexture == nullptr)
        {
            // no swapchain texture available; submit and continue
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
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);

        // draw calls go here
        SDL_BindGPUGraphicsPipeline(renderPass, graphicsPipeline);

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
     * @brief Shut down the application and release GPU and UI resources.
     *
     * Shuts down the ImGui SDL3 and SDLGPU backends, destroys the ImGui context,
     * releases the vertex and fragment GPU shaders and the graphics pipeline, and
     * destroys the SDL window.
     *
     * @param result Application exit result code provided by the SDL app framework.
     */
    virtual void quit(SDL_AppResult result) override
    {
        (void)result;

        SDL_GPUDevice* device = get_device_or_log();

        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
        ImGui::DestroyContext();

        if (device)
        {
            if (vertexShader)
            {
                SDL_ReleaseGPUShader(device, vertexShader);
                vertexShader = nullptr;
            }
            if (fragmentShader)
            {
                SDL_ReleaseGPUShader(device, fragmentShader);
                fragmentShader = nullptr;
            }
            if (graphicsPipeline)
            {
                SDL_ReleaseGPUGraphicsPipeline(device, graphicsPipeline);
                graphicsPipeline = nullptr;
            }

            if (window)
            {
                SDL_ReleaseWindowFromGPUDevice(device, window);
            }
        }

        if (window)
        {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
    }
};

sopho::App* create_app() { return new UserApp(); }
