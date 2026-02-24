#include "render/mesh.h"
#include "render/buffer.h"

#include "log_system.h"
#include "render/interface/opengl/gl_common.h"

namespace Mint {
    MeshSource::MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform)
        : m_vertices(vertices), m_indices(indices) {
        Handle = {};
        SubMesh& submesh = m_submeshes.emplace_back();
        submesh.BaseVertex = 0;
        submesh.BaseIndex = 0;
        submesh.VertexCount = static_cast<uint32_t>(vertices.size());
        submesh.IndexCount = static_cast<uint32_t>(indices.size()) * 3u;
        submesh.Transform = transform;

        // temp, 后续应该把vertex_array移动到别的地方
        m_vertex_array = VertexArray::Create();
        m_vertex_buffer = VertexBuffer::Create(m_vertices.data(), (uint32_t)(m_vertices.size() * sizeof(Vertex)));
        // 对于animated对象, 后续需要增加处理,尽量解耦
        m_vertex_buffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float3, "a_Tangent" },
            { ShaderDataType::Float3, "a_Bitangent" },
            { ShaderDataType::Float2, "a_TexCoords" }
        });
        m_vertex_array->AddVertexBuffer(m_vertex_buffer);

        m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index)));
        m_vertex_array->SetIndexBuffer(m_index_buffer);

        m_triangle_cache[0].reserve(indices.size());
        for (const Index &index : indices) {
            m_triangle_cache[0].emplace_back(
                vertices[index.V1],
                vertices[index.V2],
                vertices[index.V3]);
        }
        // Calculate bounding box
        m_bounding_box.Min = {FLT_MAX, FLT_MAX, FLT_MAX};
        m_bounding_box.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (size_t i = 0; i < m_vertices.size(); i++) {
			const Vertex& vertex = m_vertices[i];
			m_bounding_box.Min.x = glm::min(vertex.Position.x, m_bounding_box.Min.x);
			m_bounding_box.Min.y = glm::min(vertex.Position.y, m_bounding_box.Min.y);
			m_bounding_box.Min.z = glm::min(vertex.Position.z, m_bounding_box.Min.z);
			m_bounding_box.Max.x = glm::max(vertex.Position.x, m_bounding_box.Max.x);
			m_bounding_box.Max.y = glm::max(vertex.Position.y, m_bounding_box.Max.y);
			m_bounding_box.Max.z = glm::max(vertex.Position.z, m_bounding_box.Max.z);
		}

		submesh.BoundingBox = m_bounding_box;
    }

    MeshSource::MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const std::vector<SubMesh>& submeshes)
        : m_vertices(vertices), m_indices(indices), m_submeshes(submeshes) {
        Handle = {};

        m_vertex_buffer = VertexBuffer::Create(m_vertices.data(), (uint32_t)(m_vertices.size() * sizeof(Vertex)));
        m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index)));

        // Calculate bounding box
        m_bounding_box.Min = {FLT_MAX, FLT_MAX, FLT_MAX};
        m_bounding_box.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        
        for (size_t i = 0; i < m_vertices.size(); i++) {
            const Vertex &vertex = m_vertices[i];
            m_bounding_box.Min.x = glm::min(vertex.Position.x, m_bounding_box.Min.x);
            m_bounding_box.Min.y = glm::min(vertex.Position.y, m_bounding_box.Min.y);
            m_bounding_box.Min.z = glm::min(vertex.Position.z, m_bounding_box.Min.z);
            m_bounding_box.Max.x = glm::max(vertex.Position.x, m_bounding_box.Max.x);
            m_bounding_box.Max.y = glm::max(vertex.Position.y, m_bounding_box.Max.y);
            m_bounding_box.Max.z = glm::max(vertex.Position.z, m_bounding_box.Max.z);
        }
    }

    MeshSource::~MeshSource() {
    }

    void MeshSource::DumpVertexBuffer() {

    }

    void Mesh::SetSubmeshes(const std::vector<uint32_t>& submeshes, Ref<MeshSource> mesh_source) {
        // Set Submeshes
        if (!submeshes.empty()) {
            m_submeshes = submeshes;
        } else {
            const auto& submeshes = mesh_source->GetSubMeshes();
            m_submeshes.resize(submeshes.size());
            for (uint32_t i = 0; i < submeshes.size(); i++) {
                m_submeshes[i] = i;
            }
        }
    }

    Mesh::Mesh(AssetHandle mesh_source)
        : m_mesh_source(mesh_source) {
        Handle = {};
        m_material_table = Ref<MaterialTable>::Create(0);

        if (auto mesh_source_asset = AssetManager::GetAsset<MeshSource>(mesh_source); mesh_source_asset) {
            // Set all submeshes
            SetSubmeshes({}, mesh_source_asset);

            const std::vector<AssetHandle>& mesh_material = mesh_source_asset->GetMaterial();
            for (uint32_t i = 0; i < mesh_material.size(); i++) {
                // material table 需要改为AssetHandle的接口
                m_material_table->SetMaterial(i, mesh_material[i]);
            }
        }
    }

    Mesh::Mesh(AssetHandle mesh_source, const std::vector<uint32_t>& submeshes) 
        : m_mesh_source(mesh_source) {
        Handle = {};
        m_material_table = Ref<MaterialTable>::Create(0);

        if (auto mesh_source_asset = AssetManager::GetAsset<MeshSource>(mesh_source); mesh_source_asset) {
            // Set all submeshes
            SetSubmeshes(submeshes, mesh_source_asset);

            const std::vector<AssetHandle>& mesh_material = mesh_source_asset->GetMaterial();
            for (uint32_t i = 0; i < mesh_material.size(); i++) {
                m_material_table->SetMaterial(i, mesh_material[i]);
            }
        }
    }

    // static mesh
    StaticMesh::StaticMesh(AssetHandle mesh_source)
        : m_mesh_source(mesh_source) {
        Handle = {};
        m_material_table = Ref<MaterialTable>::Create(0);

        if (auto mesh_source_asset = AssetManager::GetAsset<MeshSource>(mesh_source); mesh_source_asset) {
            // Set all submeshes
            SetSubmeshes({}, mesh_source_asset);

            const std::vector<AssetHandle>& mesh_material = mesh_source_asset->GetMaterial();
            for (uint32_t i = 0; i < mesh_material.size(); i++) {
                m_material_table->SetMaterial(i, mesh_material[i]);
            }
        }
    }

    StaticMesh::StaticMesh(AssetHandle mesh_source, const std::vector<uint32_t>& submeshes) 
        : m_mesh_source(mesh_source) {
        Handle = {};
        m_material_table = Ref<MaterialTable>::Create(0);

        if (auto mesh_source_asset = AssetManager::GetAsset<MeshSource>(mesh_source); mesh_source_asset) {
            // Set all submeshes
            SetSubmeshes(submeshes, mesh_source_asset);

            const std::vector<AssetHandle>& mesh_material = mesh_source_asset->GetMaterial();
            for (uint32_t i = 0; i < mesh_material.size(); i++) {
                m_material_table->SetMaterial(i, mesh_material[i]);
            }
        }
    }

    void StaticMesh::SetSubmeshes(const std::vector<uint32_t>& submeshes, Ref<MeshSource> mesh_source) {
        // Set Submeshes
        if (!submeshes.empty()) {
            m_submeshes = submeshes;
        } else {
            const auto& submeshes = mesh_source->GetSubMeshes();
            m_submeshes.resize(submeshes.size());
            for (uint32_t i = 0; i < submeshes.size(); i++) {
                m_submeshes[i] = i;
            }
        }
    }
}

