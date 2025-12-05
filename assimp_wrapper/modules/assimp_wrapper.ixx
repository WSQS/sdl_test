// assimp_wrapper.ixx
// Created by sophomore on 12/5/25.
//
module;
#include <cstdint>
#include <string>
#include <vector>
export module assimp_wrapper;
import data_type;
import standard_scene_renderer;
export namespace sopho
{

    struct MeshData
    {
        std::vector<VertexType> vertices;
        std::vector<std::uint32_t> indices;
    };

    std::vector<MeshData> load_model(const std::string& path);
} // namespace sopho
