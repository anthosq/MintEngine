#pragma once
#include <glm/glm.hpp>
#include "asset/asset.h"
#include "render/buffer.h"
#include "render/vertex_array.h"
#include "core/time_step.h"
#include "render/shader.h"
#include "render/material.h"

#include "math/AABB.h"



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

    struct Triangle {
        Vertex v0, v1, v2;

        Triangle(const Vertex& a, const Vertex& b, const Vertex& c)
            : v0(a), v1(b), v2(c) {}
    };

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
        uint32_t VertexCount;

        glm::mat4 Transform { 1.0f };
        glm::mat4 LocalTransform { 1.0f };
        AABB BoundingBox;

        // Debuging, need to add 
        std::string MeshName, NodeName;
        // 暂时先不考虑Bone的实现, 优先确保Mesh导入完善
        // 完善Mesh导入后为Renderer添加SubmitMesh方法
    };

    // TODO: support animated skeletons and MeshFactory
    // TODO: 分离出Resource与Instance
    // MeshResource作为Resource, Submesh作为渲染atomic unit, Mesh与StaticMesh作为Instance
    // 维护MaterialTable管理材质槽位, MaterialAsset作为资源存在

    class MeshNode {
        uint32_t Parent = 0xffffffff;
        std::vector<uint32_t> Children;
        std::vector<uint32_t> SubMeshes;

        std::string Name;
        glm::mat4 localTransform;

        bool IsRoot() const { return Parent == 0xffffffff; }

    };

    class MeshSource : public Asset {
    public:
        MeshSource() = default;
        MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform);
        // Not sure
        MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const std::vector<SubMesh>& submeshes);
        virtual ~MeshSource();

        void DumpVertexBuffer();

        // MeshSource作为资源类, 以Get Set 为主要Methods
        std::vector<SubMesh>& GetSubMeshes() { return m_submeshes; }
        const std::vector<SubMesh>& GetSubMeshes() const { return m_submeshes; }

        const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        const std::vector<Index>& GetIndices() const { return m_indices; }

        // skeleton?
        // bool HasSkeleton() const { return (bool)m_skeleton; }
        // bool IsSubmeshRigged(uint32_t submeshIndex) const { return m_submeshes[submeshIndex].IsRigged(); }
        // const Skeleton* GetSkeleton() const { return m_skeleton.get(); }
        // const glm::mat4& GetSkeletonTransform() const { return m_skeleton_transform; }
        // bool IsCompatibleSkeleton(const std::string_view animation_name, const Skeleton& skeleton) const {}

        // Animation?
        // std::vector<std::string> GetAnimationNames() const {}
        // const Animation* GetAnimation(const std::string_view animation_name, const Skeleton& skeleton, ...) const {}
        // const std::vector<BoneInfluence>& GetBoneInfluences() const { return m_bone_influences; }


        std::vector<AssetHandle>& GetMaterial() { return m_materials; }
        const std::vector<AssetHandle>& GetMaterial() const { return m_materials; }

        inline const std::filesystem::path &GetFilePath() const { return m_filepath; }

        Ref<VertexBuffer> GetVertexBuffer() { return m_vertex_buffer; }
        Ref<IndexBuffer> GetIndexBuffer() { return m_index_buffer; }
        Ref<VertexArray> GetVertexArray() { return m_vertex_array; }

        const AABB& GetBoundingBox() const { return m_bounding_box; }

        const std::vector<Triangle> GetTrianglesCache(uint32_t submesh_index) {return m_triangle_cache.at(submesh_index); }

        const std::vector<MeshNode>& GetMeshNodes() const { return m_nodes; }
        const MeshNode& GetRootNode() const { return m_nodes[0]; }

    private:
        friend class AssimpMeshImporter;

        std::vector<SubMesh> m_submeshes;
        // std::unique_ptr<Assimp::Importer> m_importer;

        Ref<VertexBuffer> m_vertex_buffer;
        Ref<IndexBuffer> m_index_buffer;

        // temp
        Ref<VertexArray> m_vertex_array;

        std::vector<Vertex> m_vertices;
        std::vector<Index> m_indices;

        // Ref<VertexBuffer> m_bone_influence_Buffer;
        // std::vector<BoneInfluence> m_bone_influence;
        // std::vector<BoneInfo> m_bone_info;

        // skeleton?
        // Scope<Skeleton> m_skeleton;
        // glm::mat4 m_skeleton_transform;
        
        // animation?
        // std::vector<std::string> m_animation_names;
        // std::unordered_map<std::string, Scope<Animation>> m_animations;

        // Using UUID?
        std::vector<AssetHandle> m_materials;

        // AABB
        AABB m_bounding_box;

        std::filesystem::path m_filepath;
        
        std::unordered_map<uint32_t, std::vector<Triangle>> m_triangle_cache;
        // Nodes?
        std::vector<MeshNode> m_nodes;
    };

    class Mesh : public Asset {
    public:
        explicit Mesh(AssetHandle mesh_source);
        Mesh(AssetHandle mesh_source, const std::vector<uint32_t>& submeshes);
        virtual ~Mesh() = default;


        // pass in empty vector to set All submeshes for MeshSource
        void SetSubmeshes(const std::vector<uint32_t>& submeshes, Ref<MeshSource> mesh_source);
        const std::vector<uint32_t>& GetSubmeshes() const { return m_submeshes; }

        void SetMeshSource(AssetHandle mesh_source) { m_mesh_source = mesh_source; }
        AssetHandle GetMeshSource() const { return m_mesh_source; }

        Ref<MaterialTable> GetMaterialTable() const { return m_material_table; }

        // const Skeleton* GetSkeleton() const;
        // leave for Asset

    private:
        AssetHandle m_mesh_source;
        std::vector<uint32_t> m_submeshes;
        Ref<MaterialTable> m_material_table;

        // bool generate_colliders = false;

    };


    // StaticMesh 
    class StaticMesh : public Asset{
    public:
        explicit StaticMesh(AssetHandle mesh_source);
        StaticMesh(AssetHandle mesh_source, const std::vector<uint32_t>& submeshes);
        virtual ~StaticMesh() = default;


        // pass in empty vector to set All submeshes for MeshSource
        void SetSubmeshes(const std::vector<uint32_t>& submeshes, Ref<MeshSource> mesh_source);
        const std::vector<uint32_t>& GetSubmeshes() const { return m_submeshes; }

        void SetMeshSource(AssetHandle mesh_source) { m_mesh_source = mesh_source; }
        AssetHandle GetMeshSource() const { return m_mesh_source; }

        Ref<MaterialTable> GetMaterialTable() const { return m_material_table; }

    private:
        AssetHandle m_mesh_source;
        std::vector<uint32_t> m_submeshes;
        Ref<MaterialTable> m_material_table;

    };
}