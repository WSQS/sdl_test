// standard_scene_renderer.ixx
// Created by sophomore on 12/4/25.
//
module;
#include <cstdint>
#include <span>
#include <string>
#include <vector>
export module standard_scene_renderer;
import logos;
import primitive_renderer;
export namespace sopho
{
    // see: https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader for uniform layout
    std::string scene_vertex =
        R"WSQ(#version 460
#extension GL_KHR_vulkan_glsl : enable
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec3 v_pos;
layout (location = 2) out vec2 v_uv;

layout(std140, set = 1, binding = 0) uniform Camera
{
    mat4 uView;
    mat4 uProjection;
    vec3 uCameraPos; // Matches CameraMatrices::location
};

layout(std140, set = 1, binding = 1) uniform Object
{
    mat4 uModel;
};

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(a_position, 1.0f);
    v_normal = normalize(mat3(transpose(inverse(uModel))) * a_normal);
    v_pos = vec3(uModel * vec4(a_position, 1.0));
    v_uv = a_uv;
})WSQ";

    std::string scene_fragment =
    R"WSQ(#version 460
#extension GL_KHR_vulkan_glsl : enable
layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec3 v_pos;
layout (location = 2) in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

layout(std140, set = 3, binding = 2) uniform SceneContex {
    vec3 lightPos;
    vec3 viewPos;
};

layout(std140, set = 3, binding = 3) uniform Material {
    vec4 baseColorFactor;
    float roughness;
    float metallic;
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


    std::string scene_light_fragment =
        R"WSQ(#version 460
#extension GL_KHR_vulkan_glsl : enable
layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec3 v_pos;
layout (location = 2) in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

layout(std140, set = 3, binding = 2) uniform SceneContex {
    vec3 lightPos;
    vec3 viewPos;
};

layout(std140, set = 3, binding = 3) uniform Material {
    vec4 baseColorFactor;
    float roughness;
    float metallic;
};
void main()
{
    FragColor = vec4(1,1,1,1);
})WSQ";

    struct VertexType
    {
        float x{}, y{}, z{};
        float nx{}, ny{}, nz{};
        float u{}, v{};
    };

    struct Mesh
    {
        BufferHandle vertex_buffer;
        BufferHandle index_buffer;
        std::uint32_t index_count;
    };

    /**
     * Binding = 0
     */
    struct CameraMatrices
    {
        Mat<float, 4, 4> view{};
        Mat<float, 4, 4> projection{};
        Mat<float, 1, 3> location{};
    };
    /**
     * Binding = 2
     */
    struct SceneContex
    {
        Mat<float, 1, 4> light_pos{};
        Mat<float, 1, 4> view_pos{};
    };
    struct StandardMaterial
    {
        RenderProcedureHandle pipeline{};
        TextureHandle albedo_map{};

        /**
         * Binding 3
         */
        struct Params
        {
            Mat<float, 1, 4> base_color_factor{};
            float roughness{};
            float metallic{};
            float padding[2]{};
        } params;
    };
    class StandardSceneRenderer
    {

        struct RenderCommand
        {
            Mesh mesh{};
            StandardMaterial material{};
            /**
             * Binding = 1
             */
            Mat<float, 4, 4> model_matrix{};
        };
        std::vector<RenderCommand> m_render_queue{};

        PrimitiveRenderer* m_primitive_renderer{};
        CameraMatrices m_camera_matrices{};
        SceneContex m_scene_context{};
        template <typename T>
        static std::span<const std::byte> as_bytes(const T& value)
        {
            return std::as_bytes(std::span<const T, 1>(&value, 1));
        }

    public:
        StandardSceneRenderer& set_primitive_renderer(PrimitiveRenderer* primitive_renderer)
        {
            m_primitive_renderer = primitive_renderer;
            return *this;
        }
        StandardSceneRenderer& begin_scene(const CameraMatrices& camera_matrices, const SceneContex& scene_context)
        {
            m_camera_matrices = camera_matrices;
            m_scene_context = scene_context;
            return *this;
        }
        StandardSceneRenderer& end_scene()
        {
            m_primitive_renderer->begin_frame();
            m_primitive_renderer->begin_render_pass({.clear = true, .depth = true});
            for (const auto& render_command : m_render_queue)
            {
                m_primitive_renderer->bind_render_procedure(render_command.material.pipeline);
                m_primitive_renderer->push_vertex_uniform(
                    UniformDescriptor{.slot_index = 0, .data = as_bytes(m_camera_matrices)});
                m_primitive_renderer->bind_texture(render_command.material.albedo_map);
                m_primitive_renderer->bind_vertex_buffer(render_command.mesh.vertex_buffer);
                m_primitive_renderer->bind_index_buffer(render_command.mesh.index_buffer);
                m_primitive_renderer->push_vertex_uniform(
                    UniformDescriptor{.slot_index = 1, .data = as_bytes(render_command.model_matrix)});
                m_primitive_renderer->push_fragment_uniform(
                    UniformDescriptor{.slot_index = 2, .data = as_bytes(m_scene_context)});
                m_primitive_renderer->push_fragment_uniform(
                    UniformDescriptor{.slot_index = 3, .data = as_bytes(render_command.material.params)});
                m_primitive_renderer->draw_index(static_cast<std::int32_t>(render_command.mesh.index_count));
            }
            m_render_queue.clear();
            m_primitive_renderer->end_render_pass();
            m_primitive_renderer->end_frame();
            return *this;
        }
        StandardSceneRenderer& submit(const Mesh& mesh, const StandardMaterial& material,
                                      const Mat<float, 4, 4>& model_matrix)
        {
            m_render_queue.emplace_back(
                RenderCommand{.mesh = mesh, .material = material, .model_matrix = model_matrix});
            return *this;
        }
    };
} // namespace sopho
