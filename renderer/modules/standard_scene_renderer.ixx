// standard_scene_renderer.ixx
// Created by sophomore on 12/4/25.
//
module;
#include <cstdint>
export module standard_scene_renderer;
import logos;
import primitive_renderer;
namespace sopho
{
    struct VertexType
    {
        float x{}, y{}, z{};
        float nx{}, ny{}, nz{};
        float u{}, v{};
    };
    struct CameraMatrices
    {
        Mat<float, 4, 4> view{};
        Mat<float, 4, 4> projection{};
        Mat<float, 1, 3> location{};
    };
    struct Mesh
    {
        BufferHandle vertex_buffer;
        BufferHandle index_buffer;
        std::uint32_t index_count;
    };
    struct StandardMaterial
    {
        RenderProcedureHandle pipeline;
        TextureHandle albedo_map;

        struct Params
        {
            Mat<float, 1, 4> base_color_factor;
            float roughness;
            float metallic;
            float padding[2];
        } params;
    };
    class StandardSceneRenderer
    {
        PrimitiveRenderer* m_primitive_renderer{};
        CameraMatrices m_camera_matrices{};

    public:
        StandardSceneRenderer& set_primitive_renderer(PrimitiveRenderer* primitive_renderer)
        {
            m_primitive_renderer = primitive_renderer;
            return *this;
        }
        StandardSceneRenderer& begin_scene(const CameraMatrices& camera_matrices)
        {
            m_camera_matrices = camera_matrices;
            return *this;
        }
        StandardSceneRenderer& end_scene() { return *this; }
        StandardSceneRenderer& submit(const Mesh& mesh, const StandardMaterial& material,
                                      const Mat<float, 4, 4>& model_matrix)
        {

            return *this;
        }
    };
} // namespace sopho
