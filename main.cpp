// main.cpp
// Created by wsqsy on 11/06/2025.
//
#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <iostream>
#include <memory>
#include <numbers>
#include <string>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include "misc/cpp/imgui_stdlib.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_keycode.h"

import sdl_wrapper;

struct CameraUniform
{
    std::array<float, 16> m{};
};

class UserApp : public sopho::App
{
    // GPU + resources
    std::shared_ptr<sopho::GpuWrapper> m_gpu{};
    std::expected<sopho::RenderProcedural, sopho::GpuError> m_render_procedural{
        std::unexpected(sopho::GpuError::UNINITIALIZED)};
    std::expected<sopho::RenderData, sopho::GpuError> m_render_data{
        std::unexpected(sopho::GpuError::UNINITIALIZED)};

    // camera state
    float yaw = 0.0f;
    float pitch = 0.0f;

    CameraUniform cam{};

    std::string vertex_source =
        R"WSQ(#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 0) out vec4 v_color;

layout(std140, set = 1, binding = 0) uniform Camera
{
    mat4 uView;
};

void main()
{
  gl_Position = uView * vec4(a_position, 1.0f);
  v_color = vec4(1);
})WSQ";

    std::string fragment_source =
        R"WSQ(#version 460

layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = v_color;
})WSQ";

public:
    /**
     * @brief Initialize application resources, GPU pipeline, vertex data, and Dear ImGui.
     *
     * Creates the GPU device, window/pipeline/buffer wrappers, compiles shaders,
     * uploads the initial vertex data, initializes the camera to identity,
     * and sets up Dear ImGui and its SDL3/SDLGPU backends.
     */
    SDL_AppResult init(int argc, char** argv) override
    {
        // 1. Create GPU wrapper (device + window + claim), monadic style.
        auto gpu_result = sopho::GpuWrapper::create();
        if (!gpu_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create GpuWrapper, error = %d",
                         static_cast<int>(gpu_result.error()));
            return SDL_APP_FAILURE;
        }
        m_gpu = std::move(gpu_result.value());

        // 2. Create pipeline wrapper.
        auto pw_result = m_gpu->create_render_procedural();
        if (!pw_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create pipeline wrapper, error = %d",
                         static_cast<int>(pw_result.error()));
            return SDL_APP_FAILURE;
        }
        m_render_procedural.emplace(std::move(pw_result.value()));

        // 4. Compile shaders and build initial pipeline.
        auto pipeline_init =
            m_render_procedural.and_then([&](auto& pipeline) { return pipeline.set_vertex_shader(vertex_source); })
                .and_then([&](std::monostate) { return m_render_procedural->set_fragment_shader(fragment_source); })
                .and_then([&](std::monostate) { return m_render_procedural->submit(); });

        if (!pipeline_init)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to initialize pipeline, error = %d",
                         static_cast<int>(pipeline_init.error()));
            return SDL_APP_FAILURE;
        }

        // 3. Create vertex buffer.
        m_render_data =
            std::move(m_gpu->create_data(m_render_procedural.value(),3));
        if (!m_render_data)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create vertex buffer, error = %d",
                         static_cast<int>(m_render_data.error()));
            return SDL_APP_FAILURE;
        }

        // 5. Upload initial vertex data.
        auto upload_result = m_render_data.and_then(
            [&](auto& vertex_buffer)
            { return vertex_buffer.buffer().upload(); });

        if (!upload_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to upload initial vertex data, error = %d",
                         static_cast<int>(upload_result.error()));
            return SDL_APP_FAILURE;
        }

        // 6. Initialize camera matrix to identity.
        {
            cam.m.fill(0.0F);
            cam.m[0] = 1.0F;
            cam.m[5] = 1.0F;
            cam.m[10] = 1.0F;
            cam.m[15] = 1.0F;
        }

        // 7. Setup Dear ImGui context.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        // 8. Initialize ImGui SDL3 backend.
        if (SDL_Window* window = m_gpu->window())
        {
            ImGui_ImplSDL3_InitForSDLGPU(window);
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "GpuWrapper::window() returned null; ImGui SDL3 backend not initialized");
            return SDL_APP_FAILURE;
        }

        // 9. Initialize ImGui SDLGPU backend.
        auto format_result = m_gpu->get_texture_format();
        if (!format_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to get swapchain texture format, error = %d",
                         static_cast<int>(format_result.error()));
            return SDL_APP_FAILURE;
        }

        ImGui_ImplSDLGPU3_InitInfo init_info{};
        init_info.Device = m_gpu->device();
        init_info.ColorTargetFormat = format_result.value();
        init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
        init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
        init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;

        ImGui_ImplSDLGPU3_Init(&init_info);

        return SDL_APP_CONTINUE;
    }

    /**
     * @brief Advance the UI frame, present editors for triangle vertices and shader sources.
     */
    SDL_AppResult tick()
    {
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Begin("Editor");
        static int current = 0;
        std::array<const char*, 3> items = {"Node", "Vertex", "Fragment"};
        ImGui::Combo("##Object", &current, items.data(), static_cast<int>(items.size()));

        switch (current)
        {
        case 0: // Vertex positions
            {
                bool changed = false;
                changed |= ImGui::DragFloat3("##node1", reinterpret_cast<float*>(m_render_data.value().buffer().cpu_buffer()),
                                             0.01f, -1.f, 1.f);
                changed |= ImGui::DragFloat3(
                    "##node2", reinterpret_cast<float*>(m_render_data.value().buffer().cpu_buffer()) + 7, 0.01f, -1.f, 1.f);
                changed |= ImGui::DragFloat3(
                    "##node3", reinterpret_cast<float*>(m_render_data.value().buffer().cpu_buffer()) + 14, 0.01f, -1.f, 1.f);

                if (changed)
                {
                    auto upload_result = m_render_data.and_then(
                        [&](auto& vertex_buffer)
                        {
                            return vertex_buffer.buffer().upload();
                        });
                    if (!upload_result)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to upload vertex buffer in tick(), error = %d",
                                     static_cast<int>(upload_result.error()));
                    }
                }
            }
            break;

        case 1: // Vertex shader editor
            {
                auto line_count = std::count(vertex_source.begin(), vertex_source.end(), '\n');
                ImVec2 size(ImGui::GetContentRegionAvail().x,
                            std::min(ImGui::GetTextLineHeight() * (line_count + 3), ImGui::GetContentRegionAvail().y));

                if (ImGui::InputTextMultiline("##vertex editor", &vertex_source, size,
                                              ImGuiInputTextFlags_AllowTabInput))
                {
                    auto result = m_render_procedural.and_then(
                        [&](auto& pipeline_wrapper) { return pipeline_wrapper.set_vertex_shader(vertex_source); });
                    if (!result)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to set vertex shader from editor, error = %d",
                                     static_cast<int>(result.error()));
                    }
                }
            }
            break;

        case 2: // Fragment shader editor
            {
                auto line_count = std::count(fragment_source.begin(), fragment_source.end(), '\n');
                ImVec2 size(ImGui::GetContentRegionAvail().x,
                            std::min(ImGui::GetTextLineHeight() * (line_count + 3), ImGui::GetContentRegionAvail().y));

                if (ImGui::InputTextMultiline("##fragment editor", &fragment_source, size,
                                              ImGuiInputTextFlags_AllowTabInput))
                {
                    auto result = m_render_procedural.and_then(
                        [&](auto& pipeline_wrapper) { return pipeline_wrapper.set_fragment_shader(fragment_source); });
                    if (!result)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to set fragment shader from editor, error = %d",
                                     static_cast<int>(result.error()));
                    }
                }
            }
            break;

        default:
            break;
        }

        ImGui::End();
        ImGui::EndFrame();
        return SDL_APP_CONTINUE;
    }

    /**
     * @brief Render the triangle and ImGui UI to the GPU and present the swapchain frame.
     */
    SDL_AppResult draw()
    {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        // Rebuild pipeline if needed.
        auto pipeline_submit =
            m_render_procedural.and_then([](auto& pipeline_wrapper) { return pipeline_wrapper.submit(); });
        if (!pipeline_submit)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Pipeline submit failed, error = %d",
                         static_cast<int>(pipeline_submit.error()));
            // Upload pipeline failed, no need to draw.
            return SDL_APP_CONTINUE;
        }

        SDL_GPUDevice* device = m_gpu->device();
        if (!device)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "GpuWrapper::device() returned null in draw()");
            return SDL_APP_CONTINUE;
        }

        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
        if (!commandBuffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to acquire GPU command buffer");
            return SDL_APP_CONTINUE;
        }

        SDL_GPUTexture* swapchainTexture = nullptr;
        Uint32 width = 0, height = 0;

        SDL_Window* window = m_gpu->window();
        if (!window)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "GpuWrapper::window() returned null in draw()");
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return SDL_APP_CONTINUE;
        }

        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, &width, &height))
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to acquire swapchain texture: %s", SDL_GetError());
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return SDL_APP_CONTINUE;
        }

        if (swapchainTexture == nullptr)
        {
            // You must always submit the command buffer, even if no texture is available.
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return SDL_APP_CONTINUE;
        }

        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, commandBuffer);

        // Create the color target.
        SDL_GPUColorTargetInfo colorTargetInfo{};
        colorTargetInfo.clear_color = {240 / 255.0F, 240 / 255.0F, 240 / 255.0F, 255 / 255.0F};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = swapchainTexture;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);

        // Bind pipeline if available.
        m_render_procedural.and_then(
            [&](auto& pipeline_wrapper) -> std::expected<std::monostate, sopho::GpuError>
            {
                SDL_BindGPUGraphicsPipeline(renderPass, pipeline_wrapper.data());
                return std::monostate{};
            });

        // Compute camera matrix and upload as a vertex uniform.
        {
            float cy = std::cos(yaw);
            float sy = std::sin(yaw);
            float cp = std::cos(pitch);
            float sp = std::sin(pitch);

            std::array<float, 16> Ry = {cy,  0.0F, sy, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F,
                                        -sy, 0.0F, cy, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F};

            std::array<float, 16> Rx = {1.0F, 0.0F, 0.0F, 0.0F, 0.0F, cp,   sp,   0.0F,
                                        0.0F, -sp,  cp,   0.0F, 0.0F, 0.0F, 0.0F, 1.0F};

            auto mulMat4 = [](const float* A, const float* B, float* C)
            {
                for (int col = 0; col < 4; ++col)
                {
                    for (int row = 0; row < 4; ++row)
                    {
                        C[col * 4 + row] = A[0 * 4 + row] * B[col * 4 + 0] + A[1 * 4 + row] * B[col * 4 + 1] +
                            A[2 * 4 + row] * B[col * 4 + 2] + A[3 * 4 + row] * B[col * 4 + 3];
                    }
                }
            };

            // uView = Rx * Ry
            mulMat4(Rx.data(), Ry.data(), cam.m.data());
            SDL_PushGPUVertexUniformData(commandBuffer, 0, cam.m.data(), static_cast<std::uint32_t>(sizeof(cam.m)));
        }

        // Bind vertex buffer and draw.
        m_render_data.and_then(
            [&](auto& vertex_buffer) -> std::expected<std::monostate, sopho::GpuError>
            {
                SDL_GPUBufferBinding bufferBindings[1]{};
                bufferBindings[0].buffer = vertex_buffer.buffer().gpu_buffer();
                bufferBindings[0].offset = 0;

                SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1);
                return std::monostate{};
            });

        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, commandBuffer, renderPass);

        SDL_EndGPURenderPass(renderPass);
        SDL_SubmitGPUCommandBuffer(commandBuffer);

        return SDL_APP_CONTINUE;
    }

    SDL_AppResult iterate() override
    {
        auto result = tick();
        if (result == SDL_APP_CONTINUE)
        {
            result = draw();
        }
        return result;
    }

    SDL_AppResult event(SDL_Event* event) override
    {
        ImGui_ImplSDL3_ProcessEvent(event);
        if (event->type == SDL_EVENT_KEY_DOWN)
        {
            switch (event->key.key)
            {
            case SDLK_UP:
                pitch += 0.1F;
                pitch = std::clamp<float>(pitch, -std::numbers::pi_v<float> / 2, +std::numbers::pi_v<float> / 2);
                break;
            case SDLK_DOWN:
                pitch -= 0.1F;
                pitch = std::clamp<float>(pitch, -std::numbers::pi_v<float> / 2, +std::numbers::pi_v<float> / 2);
                break;
            case SDLK_LEFT:
                yaw += 0.1F;
                break;
            case SDLK_RIGHT:
                yaw -= 0.1F;
                break;
            default:
                break;
            }
        }

        if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            return SDL_APP_SUCCESS;
        }

        return SDL_APP_CONTINUE;
    }

    void quit(SDL_AppResult result) override
    {
        (void)result;
        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
        ImGui::DestroyContext();
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
