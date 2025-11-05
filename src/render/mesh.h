#pragma once
#include <glm/glm.hpp>
#include "asset.h"
#include "render/buffer.h"
#include "core/time_step.h"
#include "render/shader.h"
#include "render/material.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Mint {
    // temporary mesh class
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Binormal;
        glm::vec2 TexCoord;
    };


    struct Index
    {
        uint32_t V1, V2, V3;
    };

    class SubMesh {
    public:
        uint32_t BaseVertex;
        uint32_t BaseIndex;
        uint32_t MaterialIndex;
        uint32_t IndexCount;
        uint32_t VertexCount;

        // World Transform
        glm::mat4 Transform;
        glm::mat4 LocalTransform;
    };

    // TODO: support animated skeletons and MeshFactory
    class Mesh : public Asset
    {
    public:
        Mesh(const std::filesystem::path &filepath);
        ~Mesh();

        // not sure
        void OnUpdate(TimeStep ts);
        void OnImGuiRender();

        Ref<Shader> GetShader() const { return m_mesh_shader; }
        Ref<Material> GetMaterial() const { return m_material; }
        inline const std::filesystem::path &GetFilePath() const { return m_filepath; }

    private:
        std::vector<SubMesh> m_submeshes;
        std::unique_ptr<Assimp::Importer> m_importer;

        glm::mat4 m_inverse_transform;

        std::vector<Vertex> m_vertices;
        std::vector<Index> m_indices;
        const aiScene* m_scene;

        Ref<Shader> m_mesh_shader;
        Ref<Material> m_material;

        std::filesystem::path m_filepath;
    };
}