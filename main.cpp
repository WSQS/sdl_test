#include <array>
#include <iostream>
#include <optional>
#include "shaderc/shaderc.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include "misc/cpp/imgui_stdlib.h"

#include <SDL3/SDL.h>

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
    std::optional<sopho::BufferWrapper> vertex_buffer;
    std::optional<sopho::PipelineWrapper> pipeline_wrapper;
    std::shared_ptr<sopho::GpuWrapper> gpu_wrapper{std::make_shared<sopho::GpuWrapper>()};

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
     * @brief Initialize the application: create window, compile shaders, create GPU pipeline and resources, and
     * initialize ImGui.
     *
     * @param argc Number of command-line arguments provided to the application.
     * @param argv Command-line argument vector provided to the application.
     * @return SDL_AppResult SDL_APP_CONTINUE to proceed with the main loop, SDL_APP_SUCCESS to request termination.
     */
    virtual SDL_AppResult init(int argc, char** argv) override
    {
        // create a window
        window = SDL_CreateWindow("Hello, Triangle!", 960, 540, SDL_WINDOW_RESIZABLE);

        SDL_ClaimWindowForGPUDevice(gpu_wrapper->data(), window);

        options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
        auto result = compiler.CompileGlslToSpv(vertex_source, shaderc_glsl_vertex_shader, "test.glsl", options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[shaderc] compile error in test.glsl: %s",
                         result.GetErrorMessage().data());
        }

        // load the vertex shader code
        std::vector<uint32_t> vertexCode{result.cbegin(), result.cend()};

        // create the vertex shader
        SDL_GPUShaderCreateInfo vertexInfo{};
        vertexInfo.code = (Uint8*)vertexCode.data();
        vertexInfo.code_size = vertexCode.size() * 4;
        vertexInfo.entrypoint = "main";
        vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        vertexInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
        vertexInfo.num_samplers = 0;
        vertexInfo.num_storage_buffers = 0;
        vertexInfo.num_storage_textures = 0;
        vertexInfo.num_uniform_buffers = 0;

        vertexShader = SDL_CreateGPUShader(gpu_wrapper->data(), &vertexInfo);

        result = compiler.CompileGlslToSpv(fragment_source, shaderc_glsl_fragment_shader, "test.frag", options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[shaderc] compile error in est.frag: %s",
                         result.GetErrorMessage().data());
        }

        // load the fragment shader code
        std::vector<uint32_t> fragmentCode{result.cbegin(), result.cend()};

        // create the fragment shader
        SDL_GPUShaderCreateInfo fragmentInfo{};
        fragmentInfo.code = (Uint8*)fragmentCode.data();
        fragmentInfo.code_size = fragmentCode.size() * 4;
        fragmentInfo.entrypoint = "main";
        fragmentInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        fragmentInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        fragmentInfo.num_samplers = 0;
        fragmentInfo.num_storage_buffers = 0;
        fragmentInfo.num_storage_textures = 0;
        fragmentInfo.num_uniform_buffers = 0;

        fragmentShader = SDL_CreateGPUShader(gpu_wrapper->data(), &fragmentInfo);

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
        colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(gpu_wrapper->data(), window);

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

        // create the pipeline
        graphicsPipeline = SDL_CreateGPUGraphicsPipeline(gpu_wrapper->data(), &pipelineInfo);

        // create the vertex buffer
        SDL_GPUBufferCreateInfo bufferInfo{};
        bufferInfo.size = sizeof(vertices);
        bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        vertex_buffer.emplace(gpu_wrapper, &bufferInfo);

        vertex_buffer->upload(&vertices, sizeof(vertices), 0);

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
     * @brief Advance the application by one frame: update ImGui, handle UI for vertex editing and live shader
     * recompilation, record GPU commands to render the triangle and ImGui, and submit the GPU command buffer.
     *
     * Performs per-frame UI updates (including draggable vertex positions and a multiline shader editor), uploads
     * vertex data when modified, recompiles and replaces the vertex shader and graphics pipeline on shader edits,
     * acquires a GPU command buffer and the swapchain texture, executes a render pass that draws the triangle and ImGui
     * draw lists, and submits the command buffer.
     *
     * @return SDL_AppResult SDL_APP_CONTINUE to continue the main loop.
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
                vertex_buffer->upload(&vertices, sizeof(vertices), 0);
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
                    std::cerr << "[shaderc] compile error in " << "test.glsl" << ":\n"
                              << result.GetErrorMessage() << std::endl;
                }
                else
                {
                    // load the vertex shader code
                    std::vector<uint32_t> vertexCode{result.cbegin(), result.cend()};

                    // create the vertex shader
                    SDL_GPUShaderCreateInfo vertexInfo{};
                    vertexInfo.code = (Uint8*)vertexCode.data();
                    vertexInfo.code_size = vertexCode.size() * 4;
                    vertexInfo.entrypoint = "main";
                    vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
                    vertexInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
                    vertexInfo.num_samplers = 0;
                    vertexInfo.num_storage_buffers = 0;
                    vertexInfo.num_storage_textures = 0;
                    vertexInfo.num_uniform_buffers = 0;


                    SDL_ReleaseGPUShader(gpu_wrapper->data(), vertexShader);
                    vertexShader = SDL_CreateGPUShader(gpu_wrapper->data(), &vertexInfo);

                    pipelineInfo.vertex_shader = vertexShader;
                    SDL_ReleaseGPUGraphicsPipeline(gpu_wrapper->data(), graphicsPipeline);
                    graphicsPipeline = SDL_CreateGPUGraphicsPipeline(gpu_wrapper->data(), &pipelineInfo);
                }
            }

            ImGui::End();
        }

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

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
        SDL_BindGPUGraphicsPipeline(renderPass, graphicsPipeline);

        // bind the vertex buffer
        SDL_GPUBufferBinding bufferBindings[1];
        bufferBindings[0].buffer = vertex_buffer->data(); // index 0 is slot 0 in this example
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
     * @brief Cleanly shuts down the application and releases GPU and UI resources.
     *
     * Performs final cleanup: shuts down ImGui SDL/SDLGPU backends, destroys the ImGui context,
     * releases the vertex buffer, shaders, and graphics pipeline, and destroys the SDL window.
     *
     * @param result The application's exit result code provided by the SDL app framework.
     */
    virtual void quit(SDL_AppResult result) override
    {
        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
        ImGui::DestroyContext();

        // Release the shader
        SDL_ReleaseGPUShader(gpu_wrapper->data(), vertexShader);
        SDL_ReleaseGPUShader(gpu_wrapper->data(), fragmentShader);

        // release the pipeline
        SDL_ReleaseGPUGraphicsPipeline(gpu_wrapper->data(), graphicsPipeline);

        // destroy the window
        SDL_DestroyWindow(window);
    }
};

sopho::App* create_app() { return new UserApp(); }
