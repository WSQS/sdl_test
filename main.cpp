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
#include <span>
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
#include <chrono>


#include "stb_image.h"

import lifecycle;
import data_type;
import sdl_raii;
import glsl_reflector;
import sdl_wrapper;
import logos;
import renderer_factory;
import sdl_primitive_renderer;

struct VertexType
{
    float x{}, y{}, z{};
    float nx{}, ny{}, nz{};
    float u{}, v{};
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
    std::chrono::steady_clock::time_point m_last_time{std::chrono::steady_clock::now()};
    double m_fps_accumulator = 0.0;
    int m_fps_frames = 0;
    // GPU + resources
    sopho::SDLPrimitiveRenderer* m_primitive_renderer{};

    std::vector<std::shared_ptr<sopho::Renderable>> m_renderables{};

    sopho::ImageData m_image_data;
    std::shared_ptr<sopho::TextureWrapper> m_texture_wrapper{};
    SDL_GPUTexture* SceneDepthTexture{};

    // camera state
    float yaw = 0.0f;
    float yaw_speed = 0.0f;
    float pitch = 0.0f;
    float pitch_speed = 0.0f;

    sopho::Mat<float, 1, 3> location{};
    sopho::Mat<float, 1, 4> speed{};
    bool m_dragging{};
    float m_last_x{}, m_last_y{};

    int win_w = 0, win_h = 0;

    // see: https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader for uniform layout
    std::string vertex_source =
        R"WSQ(#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec3 v_pos;
layout (location = 2) out vec2 v_uv;

layout(std140, set = 1, binding = 0) uniform Camera
{
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
};

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(a_position, 1.0f);
    v_normal = normalize(mat3(transpose(inverse(uModel))) * a_normal);
    v_pos = vec3(uModel * vec4(a_position, 1.0));
    v_uv = a_uv;
})WSQ";

    std::string fragment_source =
        R"WSQ(#version 460

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec3 v_pos;
layout (location = 2) in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

layout(std140, set = 3, binding = 0) uniform Params {
    vec3 lightPos;
    vec3 viewPos;
};
layout(set = 2, binding = 0) uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, v_uv);
    if (FragColor.a <= 0.001)
        discard;
    vec3 lightDir = normalize(lightPos - v_pos);
    float diff = max(dot(v_normal, lightDir), 0.0);
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - v_pos);
    vec3 reflectDir = reflect(-lightDir, v_normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float specular = specularStrength * spec;
    FragColor.rgb *= 0.1 + diff + specular;
})WSQ";

    std::string fragment_source2 =
        R"WSQ(#version 460

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec3 v_pos;
layout (location = 2) in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1,1,1,1);
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
        auto c_primitive_renderer = sopho::SDLPrimitiveRenderer::create();
        if (!c_primitive_renderer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create Primitive Renderer, error = %d",
                         static_cast<int>(c_primitive_renderer.error()));
            return SDL_APP_FAILURE;
        }
        m_primitive_renderer = c_primitive_renderer.value();
        auto pipeline_handle = m_primitive_renderer->create_render_procedure(
            {.vert_shader = vertex_source, .frag_shader = fragment_source});
        auto pw_result = m_primitive_renderer->get_gpu().create_render_procedural();
        if (!pw_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create pipeline wrapper, error = %d",
                         static_cast<int>(pw_result.error()));
            return SDL_APP_FAILURE;
        }

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

        std::vector<VertexType> vertices{
            // +Z (front)  2 triangles
            {.x = 0.5f, .y = 0.5f, .z = 0.5f, .nx = 0, .ny = 0, .nz = 1, .u = 0, .v = 0},
            {.x = -0.5f, .y = 0.5f, .z = 0.5f, .nx = 0, .ny = 0, .nz = 1, .u = 1, .v = 0},
            {.x = 0.5f, .y = -0.5f, .z = 0.5f, .nx = 0, .ny = 0, .nz = 1, .u = 0, .v = 1},
            {.x = -0.5f, .y = 0.5f, .z = 0.5f, .nx = 0, .ny = 0, .nz = 1, .u = 1, .v = 0},
            {.x = -0.5f, .y = -0.5f, .z = 0.5f, .nx = 0, .ny = 0, .nz = 1, .u = 1, .v = 1},
            {.x = 0.5f, .y = -0.5f, .z = 0.5f, .nx = 0, .ny = 0, .nz = 1, .u = 0, .v = 1},

            // -Z (back)
            {.x = -0.5f, .y = 0.5f, .z = -0.5f, .nx = 0, .ny = 0, .nz = -1, .u = 0, .v = 0},
            {.x = 0.5f, .y = 0.5f, .z = -0.5f, .nx = 0, .ny = 0, .nz = -1, .u = 1, .v = 0},
            {.x = -0.5f, .y = -0.5f, .z = -0.5f, .nx = 0, .ny = 0, .nz = -1, .u = 0, .v = 1},
            {.x = 0.5f, .y = 0.5f, .z = -0.5f, .nx = 0, .ny = 0, .nz = -1, .u = 1, .v = 0},
            {.x = 0.5f, .y = -0.5f, .z = -0.5f, .nx = 0, .ny = 0, .nz = -1, .u = 1, .v = 1},
            {.x = -0.5f, .y = -0.5f, .z = -0.5f, .nx = 0, .ny = 0, .nz = -1, .u = 0, .v = 1},

            // +X (right)
            {.x = 0.5f, .y = 0.5f, .z = -0.5f, .nx = 1, .ny = 0, .nz = 0, .u = 0, .v = 0},
            {.x = 0.5f, .y = 0.5f, .z = 0.5f, .nx = 1, .ny = 0, .nz = 0, .u = 1, .v = 0},
            {.x = 0.5f, .y = -0.5f, .z = -0.5f, .nx = 1, .ny = 0, .nz = 0, .u = 0, .v = 1},
            {.x = 0.5f, .y = 0.5f, .z = 0.5f, .nx = 1, .ny = 0, .nz = 0, .u = 1, .v = 0},
            {.x = 0.5f, .y = -0.5f, .z = 0.5f, .nx = 1, .ny = 0, .nz = 0, .u = 1, .v = 1},
            {.x = 0.5f, .y = -0.5f, .z = -0.5f, .nx = 1, .ny = 0, .nz = 0, .u = 0, .v = 1},

            // -X (left)
            {.x = -0.5f, .y = 0.5f, .z = 0.5f, .nx = -1, .ny = 0, .nz = 0, .u = 0, .v = 0},
            {.x = -0.5f, .y = 0.5f, .z = -0.5f, .nx = -1, .ny = 0, .nz = 0, .u = 1, .v = 0},
            {.x = -0.5f, .y = -0.5f, .z = 0.5f, .nx = -1, .ny = 0, .nz = 0, .u = 0, .v = 1},
            {.x = -0.5f, .y = 0.5f, .z = -0.5f, .nx = -1, .ny = 0, .nz = 0, .u = 1, .v = 0},
            {.x = -0.5f, .y = -0.5f, .z = -0.5f, .nx = -1, .ny = 0, .nz = 0, .u = 1, .v = 1},
            {.x = -0.5f, .y = -0.5f, .z = 0.5f, .nx = -1, .ny = 0, .nz = 0, .u = 0, .v = 1},

            // +Y (top)
            {.x = 0.5f, .y = 0.5f, .z = -0.5f, .nx = 0, .ny = 1, .nz = 0, .u = 0, .v = 0},
            {.x = -0.5f, .y = 0.5f, .z = -0.5f, .nx = 0, .ny = 1, .nz = 0, .u = 1, .v = 0},
            {.x = 0.5f, .y = 0.5f, .z = 0.5f, .nx = 0, .ny = 1, .nz = 0, .u = 0, .v = 1},
            {.x = -0.5f, .y = 0.5f, .z = -0.5f, .nx = 0, .ny = 1, .nz = 0, .u = 1, .v = 0},
            {.x = -0.5f, .y = 0.5f, .z = 0.5f, .nx = 0, .ny = 1, .nz = 0, .u = 1, .v = 1},
            {.x = 0.5f, .y = 0.5f, .z = 0.5f, .nx = 0, .ny = 1, .nz = 0, .u = 0, .v = 1},

            // -Y (bottom)
            {.x = 0.5f, .y = -0.5f, .z = 0.5f, .nx = 0, .ny = -1, .nz = 0, .u = 0, .v = 0},
            {.x = -0.5f, .y = -0.5f, .z = 0.5f, .nx = 0, .ny = -1, .nz = 0, .u = 1, .v = 0},
            {.x = 0.5f, .y = -0.5f, .z = -0.5f, .nx = 0, .ny = -1, .nz = 0, .u = 0, .v = 1},
            {.x = -0.5f, .y = -0.5f, .z = 0.5f, .nx = 0, .ny = -1, .nz = 0, .u = 1, .v = 0},
            {.x = -0.5f, .y = -0.5f, .z = -0.5f, .nx = 0, .ny = -1, .nz = 0, .u = 1, .v = 1},
            {.x = 0.5f, .y = -0.5f, .z = -0.5f, .nx = 0, .ny = -1, .nz = 0, .u = 0, .v = 1},
        };

        std::vector<std::uint32_t> indices{};

        for (int i = 0; i < 36; ++i)
        {
            indices.push_back(i);
        }

        // 3. Create vertex buffer.
        auto render_data = sopho::RenderData::Builder{}
                               .set_vertex_layout(pw_result.value().vertex_layout())
                               .set_vertex_count(36)
                               .set_index_count(36)
                               .set_vertices(std::span(vertices))
                               .set_indices(std::span(indices))
                               .build(m_primitive_renderer->get_gpu());
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

        m_renderables.emplace_back(std::make_shared<sopho::Renderable>(sopho::Renderable{
            .m_render_procedural = pipeline_handle.value(), .m_render_data = std::move(render_data.value())}));

        auto pw_result2 = m_primitive_renderer->get_gpu().create_render_procedural();
        pipeline_init = pw_result2.and_then([&](auto& pipeline) { return pipeline.set_vertex_shader(vertex_source); })
                            .and_then([&](std::monostate) { return pw_result2->set_fragment_shader(fragment_source2); })
                            .and_then([&](std::monostate) { return pw_result2->submit(); });
        if (!pipeline_init)
        {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to initialize pipeline, error = %d",
                         static_cast<int>(pipeline_init.error()));
            return SDL_APP_FAILURE;
        }
        auto pipeline_handle2 = m_primitive_renderer->create_render_procedure(
            {.vert_shader = vertex_source, .frag_shader = fragment_source2});
        m_renderables.emplace_back(std::make_shared<sopho::Renderable>(sopho::Renderable{
            .m_render_procedural = pipeline_handle2.value(), .m_render_data = m_renderables[0]->data()}));

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
        if (SDL_Window* window = m_primitive_renderer->get_gpu().window())
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
        auto format_result = m_primitive_renderer->get_gpu().get_texture_format();
        if (!format_result)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to get swapchain texture format, error = %d",
                         static_cast<int>(format_result.error()));
            return SDL_APP_FAILURE;
        }

        ImGui_ImplSDLGPU3_InitInfo init_info{};
        init_info.Device = m_primitive_renderer->get_gpu().device();
        init_info.ColorTargetFormat = format_result.value();
        init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
        init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
        init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;

        ImGui_ImplSDLGPU3_Init(&init_info);
        m_image_data = load_image();

        auto texture =
            sopho::TextureWrapper::Builder{}.set_image_data(m_image_data).build(m_primitive_renderer->get_gpu());
        if (texture)
        {
            m_texture_wrapper = std::make_shared<sopho::TextureWrapper>(std::move(texture.value()));
        }
        else
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_GPU, "Failed to create texture: error = %d",
                        static_cast<int>(texture.error()));
        }

        SDL_GetWindowSizeInPixels(m_primitive_renderer->get_gpu().window(), &win_w, &win_h);
        SDL_GPUTextureCreateInfo ci = {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
            .width = static_cast<std::uint32_t>(win_w),
            .height = static_cast<std::uint32_t>(win_h),
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .sample_count = SDL_GPU_SAMPLECOUNT_1,
        };

        SceneDepthTexture = SDL_CreateGPUTexture(m_primitive_renderer->get_gpu().device(), &ci);
        return SDL_APP_CONTINUE;
    }

    SDL_AppResult update(float dt)
    {
        m_fps_frames++;
        m_fps_accumulator += dt;
        if (m_fps_accumulator >= 1)
        {
            SDL_Log("Fps: %f in %f s", m_fps_frames / m_fps_accumulator, m_fps_accumulator);
            m_fps_frames = 0;
            m_fps_accumulator = 0.0;
        }
        pitch += pitch_speed * dt;
        pitch = std::clamp<float>(pitch, -std::numbers::pi_v<float> / 2, +std::numbers::pi_v<float> / 2);
        yaw += yaw_speed * dt;
        location =
            location + ((sopho::rotation_x(-pitch) * sopho::rotation_y(yaw)).transpose() * speed).resize<1, 3>() * dt;
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

        // switch (current)
        // {
        // case 0: // Vertex Edit
        //     {
        //         bool changed = false;
        //         auto editor_data = m_renderable->data()->vertex_view();
        //         auto raw_ptr = editor_data.raw;
        //         for (int vertex_index = 0; vertex_index < editor_data.vertex_count; ++vertex_index)
        //         {
        //             for (const auto& format : editor_data.layout.get_vertex_reflection().inputs)
        //             {
        //                 switch (format.basic_type)
        //                 {
        //                 case sopho::BasicType::FLOAT:
        //                     {
        //                         switch (format.vector_size)
        //                         {
        //                         case 2:
        //                             changed |= ImGui::DragFloat2(std::format("{}{}", format.name,
        //                             vertex_index).data(),
        //                                                          reinterpret_cast<float*>(raw_ptr), 0.01f,
        //                                                          -1.f, 1.f);
        //                             break;
        //                         case 3:
        //                             changed |= ImGui::DragFloat3(std::format("{}{}", format.name,
        //                             vertex_index).data(),
        //                                                          reinterpret_cast<float*>(raw_ptr), 0.01f,
        //                                                          -1.f, 1.f);
        //                             break;
        //                         case 4:
        //                             changed |= ImGui::DragFloat4(std::format("{}{}", format.name,
        //                             vertex_index).data(),
        //                                                          reinterpret_cast<float*>(raw_ptr), 0.01f,
        //                                                          -1.f, 1.f);
        //                             break;
        //                         default:
        //                             SDL_Log("Not implemented size");
        //                             assert(false);
        //                             break;
        //                         }
        //                     }
        //                     break;
        //                 default:
        //                     SDL_Log("Not implemented Basic type");
        //                     assert(false);
        //                     break;
        //                 }
        //                 auto size = sopho::get_size(sopho::to_sdl_format(format.basic_type, format.vector_size));
        //                 raw_ptr += size;
        //             }
        //         }
        //         auto index_view = m_renderable->data()->index_view();
        //         auto index_ptr = index_view.raw;
        //         for (int index_index = 0; index_index < index_view.index_count; index_index += 3)
        //         {
        //             changed |= ImGui::InputInt3(std::format("index_{}", index_index).data(),
        //                                         reinterpret_cast<int*>(index_ptr));
        //             index_ptr += 3 * sizeof(int);
        //         }
        //         if (changed)
        //         {
        //             auto upload_result = m_renderable->data()->upload();
        //             if (!upload_result)
        //             {
        //                 SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to upload vertex buffer in tick(), error = %d",
        //                              static_cast<int>(upload_result.error()));
        //             }
        //         }
        //     }
        //     break;
        //
        // case 1: // Vertex shader editor
        //     {
        //         auto line_count = std::count(vertex_source.begin(), vertex_source.end(), '\n');
        //         ImVec2 size(ImGui::GetContentRegionAvail().x,
        //                     std::min(ImGui::GetTextLineHeight() * (line_count + 3),
        //                     ImGui::GetContentRegionAvail().y));
        //
        //         if (ImGui::InputTextMultiline("##vertex editor", &vertex_source, size,
        //                                       ImGuiInputTextFlags_AllowTabInput))
        //         {
        //             auto result = m_renderable->procedural()->set_vertex_shader(vertex_source);
        //             if (!result)
        //             {
        //                 SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to set vertex shader from editor, error = %d",
        //                              static_cast<int>(result.error()));
        //             }
        //             else
        //             {
        //                 auto new_data = sopho::RenderData::Builder{}
        //                                     .set_vertex_layout(m_renderable->procedural()->vertex_layout())
        //                                     .set_vertex_count(8)
        //                                     .set_index_count(36)
        //                                     .build(*m_gpu.get());
        //                 m_renderable->data() = std::move(new_data.value());
        //                 m_renderable->data()->upload();
        //             }
        //         }
        //     }
        //     break;
        //
        // case 2: // Fragment shader editor
        //     {
        //         auto line_count = std::count(fragment_source.begin(), fragment_source.end(), '\n');
        //         ImVec2 size(ImGui::GetContentRegionAvail().x,
        //                     std::min(ImGui::GetTextLineHeight() * (line_count + 3),
        //                     ImGui::GetContentRegionAvail().y));
        //
        //         if (ImGui::InputTextMultiline("##fragment editor", &fragment_source, size,
        //                                       ImGuiInputTextFlags_AllowTabInput))
        //         {
        //             auto result = m_renderable->procedural()->set_fragment_shader(fragment_source);
        //             if (!result)
        //             {
        //                 SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to set fragment shader from editor, error =
        //                 %d",
        //                              static_cast<int>(result.error()));
        //             }
        //         }
        //     }
        //     break;
        //
        // default:
        //     break;
        // }

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

        SDL_GPUDevice* device = m_primitive_renderer->get_gpu().device();
        if (!device)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "GpuWrapper::device() returned null in draw()");
            return SDL_APP_CONTINUE;
        }
        int w{}, h{};
        SDL_GetWindowSize(m_primitive_renderer->get_gpu().window(), &w, &h);
        m_primitive_renderer->begin_frame();

        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, m_primitive_renderer->get_command_buffer().raw());

        m_primitive_renderer->begin_render_pass({.clear = true, .depth = true});

        auto renderable = m_renderables[0];
        std::array<sopho::Mat<float, 4, 4>, 3> camera_mat{};
        // Model
        camera_mat[0] = sopho::translate(0.0f, -4.f, -5.0f) * sopho::rotation_y(1.6) * sopho::scale(10);
        // View
        camera_mat[1] = sopho::rotation_x(-pitch) * sopho::rotation_y(yaw) *
            sopho::translate(-location(0), -location(1), -location(2));
        // Projection`
        camera_mat[2] = sopho::perspective(1, static_cast<float>(w) / h, 0.1, 50);
        m_primitive_renderer->bind_render_procedure(renderable->procedural());
        renderable->draw(
            sopho::RenderContext{.render_pass = m_primitive_renderer->get_render_pass().raw(),
                                 .command_buffer = m_primitive_renderer->get_command_buffer().raw(),
                                 .camera_mat = camera_mat,
                                 .pos = std::array{sopho::Mat<float, 1, 4>{0.0f, 2.f, -6.0f}, location.resize<1, 4>()},
                                 .texture_wrapper = m_texture_wrapper});
        renderable = m_renderables[1];
        // Model
        camera_mat[0] = sopho::translate(0.0f, 2.f, -6.0f);
        // View
        camera_mat[1] = sopho::rotation_x(-pitch) * sopho::rotation_y(yaw) *
            sopho::translate(-location(0), -location(1), -location(2));
        // Projection
        camera_mat[2] = sopho::perspective(1, static_cast<float>(w) / h, 0.1, 50);
        m_primitive_renderer->bind_render_procedure(renderable->procedural());
        renderable->draw(sopho::RenderContext{.render_pass = m_primitive_renderer->get_render_pass().raw(),
                                              .command_buffer = m_primitive_renderer->get_command_buffer().raw(),
                                              .camera_mat = camera_mat});

        m_primitive_renderer->end_render_pass();
        m_primitive_renderer->begin_render_pass({.clear = false, .depth = false});

        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, m_primitive_renderer->get_command_buffer().raw(),
                                         m_primitive_renderer->get_render_pass().raw());

        m_primitive_renderer->end_render_pass();
        m_primitive_renderer->end_frame();
        return SDL_APP_CONTINUE;
    }

    SDL_AppResult iterate() override
    {
        auto now{std::chrono::steady_clock::now()};
        std::chrono::duration<double> delta = now - m_last_time;
        m_last_time = now;
        auto result = update(delta.count());
        if (result == SDL_APP_CONTINUE)
        {
            result = tick();
        }
        if (result == SDL_APP_CONTINUE)
        {
            result = draw();
        }
        return result;
    }

    SDL_AppResult event(SDL_Event* event) override
    {
        ImGui_ImplSDL3_ProcessEvent(event);

        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureKeyboard)
        {
            switch (event->type)
            {
            case SDL_EVENT_KEY_DOWN:
                {
                    switch (event->key.key)
                    {
                    case SDLK_UP:
                        pitch_speed = 0.5F;
                        break;
                    case SDLK_DOWN:
                        pitch_speed = -0.5F;
                        break;
                    case SDLK_LEFT:
                        yaw_speed = -0.5F;
                        break;
                    case SDLK_RIGHT:
                        yaw_speed = 0.5F;
                        break;
                    case SDLK_W:
                        speed(2) = -4.F;
                        break;
                    case SDLK_S:
                        speed(2) = 4.F;
                        break;
                    case SDLK_A:
                        speed(0) = -4.F;
                        break;
                    case SDLK_D:
                        speed(0) = 4.F;
                        break;
                    case SDLK_E:
                        speed(1) = 4.F;
                        break;
                    case SDLK_Q:
                        speed(1) = -4.F;
                        break;
                    default:
                        break;
                    }
                }
                break;
            case SDL_EVENT_KEY_UP:
                {
                    switch (event->key.key)
                    {
                    case SDLK_UP:
                        pitch_speed = 0.F;
                        break;
                    case SDLK_DOWN:
                        pitch_speed = 0.F;
                        break;
                    case SDLK_LEFT:
                        yaw_speed = 0.F;
                        break;
                    case SDLK_RIGHT:
                        yaw_speed = 0.F;
                        break;
                    case SDLK_W:
                        speed(2) = 0.F;
                        break;
                    case SDLK_S:
                        speed(2) = 0.F;
                        break;
                    case SDLK_A:
                        speed(0) = 0.F;
                        break;
                    case SDLK_D:
                        speed(0) = 0.F;
                        break;
                    case SDLK_Q:
                        speed(1) = 0.F;
                        break;
                    case SDLK_E:
                        speed(1) = 0.F;
                        break;
                    default:
                        break;
                    }
                }
                break;
            }
        }

        if (!io.WantCaptureMouse)
        {
            switch (event->type)
            {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event->button.button == SDL_BUTTON_LEFT)
                {
                    m_dragging = true;
                    m_last_x = event->button.x;
                    m_last_y = event->button.y;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event->button.button == SDL_BUTTON_LEFT)
                {
                    m_dragging = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (m_dragging)
                {
                    float x = event->motion.x;
                    float y = event->motion.y;

                    float dx = x - m_last_x;
                    float dy = y - m_last_y;

                    m_last_x = x;
                    m_last_y = y;

                    yaw += dx * 0.01;
                    pitch -= dy * 0.01;

                    pitch = std::clamp<float>(pitch, -std::numbers::pi_v<float> / 2, +std::numbers::pi_v<float> / 2);
                }
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
        SDL_ReleaseGPUTexture(m_primitive_renderer->get_gpu().device(), SceneDepthTexture);
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
 * @return sopho::App* Pointer to a heap-allocated application object; the caller takes ownership and is responsible
 * for deleting it.
 */
sopho::checkable<sopho::App*> create_app(int argc, char** argv) { return new UserApp(); }
