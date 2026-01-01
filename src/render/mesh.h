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
#define MAX_BONE_INFLUENCE 4

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        glm::vec2 TexCoords;
    };

    struct AnimatedVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;

        uint32_t BoneIDs[MAX_BONE_INFLUENCE] = {0, 0, 0, 0};
        float Weights[MAX_BONE_INFLUENCE] = { 0.0f, 0.0f, 0.0f, 0.0f };

        void AddBoneData(uint32_t BoneID, float Weight) {
            for (size_t i = 0; i < MAX_BONE_INFLUENCE; i++) {
                if (Weights[i] == 0.0f) {
                    BoneIDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }
            // should never get here - more bones than we have space for
            // assert(false && "Too many bones for vertex!");
        }
    };

    static const int num_attributes = 5;
    struct Index {
        uint32_t V1, V2, V3;
    };

    static_assert(sizeof(Index) == 3 * sizeof(uint32_t));


    // later need to consider serialization & MeshFactory
    struct BoneInfo {
        glm::mat4 BoneOffset;
        glm::mat4 FinalTransformation;
    };

    struct VertexBoneData {
        uint32_t BoneIDs[MAX_BONE_INFLUENCE];
        float Weights[MAX_BONE_INFLUENCE];

        VertexBoneData() {
            memset(BoneIDs, 0, sizeof(BoneIDs));
            memset(Weights, 0, sizeof(Weights));
        }

        void AddBoneData(uint32_t BoneID, float Weight) {
            for (size_t i = 0; i < MAX_BONE_INFLUENCE; i++) {
                if (Weights[i] == 0.0f) {
                    BoneIDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }
            // should never get here - more bones than we have space for
            // assert(false && "Too many bones for vertex!");
        }
    };


    class SubMesh {
    public:
        uint32_t BaseVertex;
        uint32_t BaseIndex;
        uint32_t MaterialIndex;
        uint32_t IndexCount;
        // uint32_t VertexCount;

        glm::mat4 Transform;
    };

    // TODO: support animated skeletons and MeshFactory
    // TODO: 分离出Resource与Instance
    // 参照UE
    // MeshResource作为Resource, Submesh作为渲染atomic unit, Mesh与StaticMesh作为Instance
    // 维护MaterialTable管理材质槽位, MaterialAsset作为资源存在
    // 注意, 需要先完成Material的设计, 然后重构Shader关于Uniform部分的内容, 再回来设计Mesh

    class Mesh : public Asset
    {
    public:
        Mesh(const std::filesystem::path &filepath);
        ~Mesh();

        // not sure
        void OnUpdate(TimeStep ts);
        void OnImGuiRender();
        void DumpVertexBuffer();

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