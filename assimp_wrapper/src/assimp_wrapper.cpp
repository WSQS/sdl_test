// assimp_wrapper.cpp
// Created by sophomore on 12/5/25.
//
module;
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstdint>
#include <string>
#include <vector>
module assimp_wrapper;
import data_type;
import standard_scene_renderer;
namespace sopho
{
    MeshData processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<VertexType> vertices;
        std::vector<std::uint32_t> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            VertexType vertex;
            vertex.x = mesh->mVertices[i].x;
            vertex.y = mesh->mVertices[i].y;
            vertex.z = mesh->mVertices[i].z;
            vertex.nx = mesh->mNormals[i].x;
            vertex.ny = mesh->mNormals[i].y;
            vertex.nz = mesh->mNormals[i].z;
            vertex.u = mesh->mTextureCoords[0][i].x;
            vertex.v = mesh->mTextureCoords[0][i].y;
            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        // TODO: Add Texture
        // if (mesh->mMaterialIndex >= 0)
        // {
        //     aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        //     std::vector<ImageData> diffuseMaps =
        //         loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        //     textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        //     std::vector<ImageData> specularMaps =
        //         loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        //     textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // }

        return MeshData(vertices, indices);
    }

    std::vector<MeshData> process_node(aiNode* node, const aiScene* scene)
    {
        // process all the node's meshes (if any)
        std::vector<MeshData> meshes;
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            meshes.append_range(process_node(node->mChildren[i], scene));
        }
        return meshes;
    }
    std::vector<MeshData> load_model(const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            return std::vector<MeshData>{};
        }
        return process_node(scene->mRootNode, scene);
    }
} // namespace sopho
