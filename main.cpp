// main.cpp
// Created by wsqsy on 11/06/2025.
//
#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <format>
#include <memory>
#include <numbers>
#include <string>
#include <variant>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include "misc/cpp/imgui_stdlib.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_keycode.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

import data_type;
import glsl_reflector;
import sdl_wrapper;

/**
 * @brief Structure to hold camera transformation matrix data.
 *
 * Contains a 4x4 matrix (16 elements) representing the camera's view transformation.
 */
struct CameraUniform
{
    std::array<float, 16> m{}; ///< 4x4 transformation matrix as a flat array
};

/**
 * @brief Loads image data from the test texture file.
 *
 * Uses stb_image library to load a PNG file (assets/test_texture.png) into an ImageData structure.
 * The image is flipped vertically on load and the pixel data is stored as a vector of bytes.
 *
 * @return sopho::ImageData Structure containing the loaded image dimensions, channels, and pixel data.
 * Returns an empty structure if loading fails.
 */
sopho::ImageData load_image()
{
    stbi_set_flip_vertically_on_load(true);
    std::string file_name{"assets/test_texture.png"};
    sopho::ImageData result;
    auto data = stbi_load(file_name.data(), &result.width, &result.height, &result.channels, 4);
    result.channels = 4;

    if (!data)
    {
        SDL_Log("stbi_load failed for %s: %s", file_name.data(), stbi_failure_reason());
    }
    else
    {
        result.pixels.assign(reinterpret_cast<std::byte*>(data),
                             reinterpret_cast<std::byte*>(data) + result.width * result.height * result.channels);
        stbi_image_free(data);
        SDL_Log("stbi_load succeeded, w: %d h:%d ch:%d", result.width, result.height, result.channels);
    }
    return result;
}

class UserApp : public sopho::App
{
    // GPU + resources
    std::shared_ptr<sopho::GpuWrapper> m_gpu{};

    std::shared_ptr<sopho::Renderable> m_renderable{};

    sopho::ImageData m_image_data;
    std::shared_ptr<sopho::TextureWrapper> m_texture_wrapper{};

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
  v_color = a_color;
})WSQ";

    std::string fragment_source =
        R"WSQ(#version 460

layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, v_color.xy);
})WSQ";

public:
    /**
     * @brief Initialize application GPU resources, shaders, vertex data, camera, and Dear ImGui.
     *
     * Performs creation of the GPU wrapper and render procedural, compiles and submits the vertex
     * and fragment shaders, creates and uploads initial render data, sets the camera uniform to the
     * identity matrix, and initializes Dear ImGui with SDL3 and SDLGPU backends.
     *
     * @return SDL_AppResult `SDL_APP_CONTINUE` on successful initialization, `SDL_APP_FAILURE` on error.
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

        // 4. Compile shaders and build initial pipeline.
        auto pipeline_init =
            pw_result.and_then([&](auto& pipeline) { return pipeline.set_vertex_shader(vertex_source); })
                .and_then([&](std::monostate) { return pw_result->set_fragment_shader(fragment_source); })
                .and_then([&](std::monostate) { return pw_result->submit(); });

        if (!pipeline_init)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to initialize pipeline, error = %d",
                         static_cast<int>(pipeline_init.error()));
            return SDL_APP_FAILURE;
        }

        // 3. Create vertex buffer.
        auto render_data = m_gpu->create_data(pw_result.value(), 4);
        if (!render_data)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create vertex buffer, error = %d",
                         static_cast<int>(render_data.error()));
            return SDL_APP_FAILURE;
        }

        // 5. Upload initial vertex data.
        auto upload_result = render_data.and_then([&](auto& vertex_buffer) { return vertex_buffer->upload(); });

        if (!upload_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to upload initial vertex data, error = %d",
                         static_cast<int>(upload_result.error()));
            return SDL_APP_FAILURE;
        }

        m_renderable = std::make_shared<sopho::Renderable>(sopho::Renderable{
            .m_render_procedural = std::make_shared<sopho::RenderProcedural>(std::move(pw_result.value())),
            .m_render_data = std::move(render_data.value())});

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
        m_image_data = load_image();

        auto texture = sopho::TextureWrapper::Builder{}.set_image_data(m_image_data).build(*m_gpu.get());
        if (texture)
        {
            m_texture_wrapper = std::make_shared<sopho::TextureWrapper>(std::move(texture.value()));
        }
        return SDL_APP_CONTINUE;
    }

    /**
     * @brief Advance the UI frame and present editors for vertex data and shader sources.
     *
     * Displays the ImGui demo and an "Editor" window with three modes:
     * - Node/Vertex editing: exposes per-vertex attributes for editing and uploads the vertex buffer when modified.
     * - Vertex shader editing: allows editing the vertex GLSL source and applies it to the procedural pipeline when
     * changed.
     * - Fragment shader editing: allows editing the fragment GLSL source and applies it to the procedural pipeline when
     * changed.
     *
     * Any failures to upload vertex data or update shaders are logged.
     *
     * @return SDL_AppResult SDL_APP_CONTINUE to indicate the application should continue running.
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
        case 0: // Vertex Edit
            {
                bool changed = false;
                auto editor_data = m_renderable->data()->vertex_view();
                auto raw_ptr = editor_data.raw;
                for (int vertex_index = 0; vertex_index < editor_data.vertex_count; ++vertex_index)
                {
                    for (const auto& format : editor_data.layout.get_vertex_reflection().inputs)
                    {
                        switch (format.basic_type)
                        {
                        case sopho::BasicType::FLOAT:
                            {
                                switch (format.vector_size)
                                {
                                case 3:
                                    changed |= ImGui::DragFloat3(std::format("{}{}", format.name, vertex_index).data(),
                                                                 reinterpret_cast<float*>(raw_ptr), 0.01f, -1.f, 1.f);
                                    break;
                                case 4:
                                    changed |= ImGui::DragFloat4(std::format("{}{}", format.name, vertex_index).data(),
                                                                 reinterpret_cast<float*>(raw_ptr), 0.01f, -1.f, 1.f);
                                }
                            }
                            break;
                        default:
                            break;
                        }
                        auto size = sopho::get_size(sopho::to_sdl_format(format.basic_type, format.vector_size));
                        raw_ptr += size;
                    }
                }
                auto index_view = m_renderable->data()->index_view();
                auto index_ptr = index_view.raw;
                for (int index_index = 0; index_index < 2; ++index_index)
                {
                    changed |= ImGui::InputInt3(std::format("index_{}", index_index).data(),
                                                reinterpret_cast<int*>(index_ptr));
                    index_ptr += 3 * sizeof(int);
                }
                if (changed)
                {
                    auto upload_result = m_renderable->data()->upload();
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
                    auto result = m_renderable->procedural()->set_vertex_shader(vertex_source);
                    if (!result)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to set vertex shader from editor, error = %d",
                                     static_cast<int>(result.error()));
                    }
                    else
                    {
                        auto new_data = m_gpu->create_data(*m_renderable->procedural(), 4);
                        m_renderable->data() = std::move(new_data.value());
                        m_renderable->data()->upload();
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
                    auto result = m_renderable->procedural()->set_fragment_shader(fragment_source);
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
     * @brief Render the scene (triangle and ImGui) into the current swapchain image and present it.
     *
     * Performs pipeline submission if needed, prepares ImGui draw data, records GPU commands
     * to clear and render the color target, uploads the camera uniform, binds vertex buffers
     * and the graphics pipeline, issues the draw call, renders ImGui, and submits the command buffer.
     *
     * @return SDL_AppResult `SDL_APP_CONTINUE` to keep the application running.
     */
    SDL_AppResult draw()
    {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        // Rebuild pipeline if needed.
        auto pipeline_submit = m_renderable->procedural()->submit();
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

        SDL_BindGPUGraphicsPipeline(renderPass, m_renderable->procedural()->data());

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

        SDL_BindGPUVertexBuffers(renderPass, 0, m_renderable->data()->get_vertex_buffer_binding().data(),
                                 m_renderable->data()->get_vertex_buffer_binding().size());

        SDL_BindGPUIndexBuffer(renderPass, &m_renderable->data()->get_index_buffer_binding(),
                               SDL_GPU_INDEXELEMENTSIZE_32BIT);
        if (m_texture_wrapper)
        {
            SDL_BindGPUFragmentSamplers(renderPass, 0, m_texture_wrapper->get(), 1);
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Texture not available, skip binding sampler");
        }


        SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);

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
sopho::checkable<sopho::App*> create_app(int argc, char** argv) { return new UserApp(); }
