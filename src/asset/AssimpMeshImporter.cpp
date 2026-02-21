#include "AssimpMeshImporter.h"
#include "asset/AssimpLogSystem.h"
#include "asset/asset.h"
#include "render/render_system.h"
#include "render/texture.h"
#include "render/material.h"
#include "log_system.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

// temporary implementation using Assimp

// Need to move the importer part to MeshImporter class or AssimpImporter class.

// temporary anonymous namespace for internal linkage
namespace Mint {
    // const unsigned int ImportFlags =
    //     aiProcess_CalcTangentSpace |
    //     aiProcess_Triangulate |
    //     aiProcess_SortByPType |
    //     aiProcess_PreTransformVertices |
    //     aiProcess_GenNormals |
    //     aiProcess_GenUVCoords |
    //     aiProcess_OptimizeMeshes |
    //     aiProcess_Debone |
    //     aiProcess_ValidateDataStructure;
    
	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace          // Create binormals/tangents just in case
		| aiProcess_Triangulate             // Make sure we're triangles
		| aiProcess_SortByPType             // Split meshes by primitive type
		| aiProcess_GenNormals              // Make sure we have legit normals
		| aiProcess_GenUVCoords             // Convert UVs if required 
//		| aiProcess_OptimizeGraph
		| aiProcess_OptimizeMeshes          // Batch draws where possible
		| aiProcess_JoinIdenticalVertices
		| aiProcess_LimitBoneWeights        // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
		| aiProcess_ValidateDataStructure   // Validation
		| aiProcess_GlobalScale             // e.g. convert cm to m for fbx import (and other formats where cm is native)
		;

    namespace Utils {
        glm::mat4 Mat4FromAIMatrix(const aiMatrix4x4& aiMat) {
            glm::mat4 result;
            result[0][0] = aiMat.a1; result[1][0] = aiMat.a2; result[2][0] = aiMat.a3; result[3][0] = aiMat.a4;
			result[0][1] = aiMat.b1; result[1][1] = aiMat.b2; result[2][1] = aiMat.b3; result[3][1] = aiMat.b4;
			result[0][2] = aiMat.c1; result[1][2] = aiMat.c2; result[2][2] = aiMat.c3; result[3][2] = aiMat.c4;
			result[0][3] = aiMat.d1; result[1][3] = aiMat.d2; result[2][3] = aiMat.d3; result[3][3] = aiMat.d4;
			return result;
        }
    }


    AssimpMeshImporter::AssimpMeshImporter(const std::filesystem::path& path)
        : m_Path(path) {
            AssimpLogStream::Initialize();
    }

    Ref<MeshSource> AssimpMeshImporter::ImportToMeshSource() {
        // Implementation for importing to MeshSource
        Ref<MeshSource> meshSource = Ref<MeshSource>::Create();
        Assimp::Importer importer;
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
        const aiScene* scene = importer.ReadFile(m_Path.string(), s_MeshImportFlags);
        if (!scene) {
            LOG_ERROR(fmt::format("Failed to load mesh from path: {}, error: {}", m_Path.string(), importer.GetErrorString()));
            meshSource->SetFlag(AssetFlag::Invalid);
            return nullptr;
        }

        // 后续实现skeleton

        if (scene->HasMeshes()) {
            uint32_t vertexCount = 0;
			uint32_t indexCount = 0;

			meshSource->m_bounding_box.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
			meshSource->m_bounding_box.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			meshSource->m_submeshes.reserve(scene->mNumMeshes);
			for (unsigned m = 0; m < scene->mNumMeshes; m++)
			{
				aiMesh* mesh = scene->mMeshes[m];

				if (!mesh->HasPositions())
				{
					LOG_ERROR(fmt::format("Mesh index {0} with name '{1}' has no vertex positions - skipping import!", m, mesh->mName.C_Str()));
				}
				if (!mesh->HasNormals())
				{
					LOG_WARN(fmt::format("Mesh index {0} with name '{1}' has no vertex normals, and they could not be computed - skipping import!", m, mesh->mName.C_Str()));
				}

				bool skip = !mesh->HasPositions() || !mesh->HasNormals();

				// still have to create a placeholder submesh even if we are skipping it (otherwise TraverseNodes() does not work)
				SubMesh& submesh = meshSource->m_submeshes.emplace_back();
				submesh.BaseVertex = vertexCount;
				submesh.BaseIndex = indexCount;
				submesh.MaterialIndex = mesh->mMaterialIndex;
				submesh.VertexCount = skip? 0 : mesh->mNumVertices;
				submesh.IndexCount = skip? 0 : mesh->mNumFaces * 3;
				submesh.MeshName = mesh->mName.C_Str();

				if (skip) continue;

				vertexCount += mesh->mNumVertices;
				indexCount += submesh.IndexCount;

				// Vertices
				auto& aabb = submesh.BoundingBox;
				aabb.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
				aabb.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					Vertex vertex;
					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
					aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
					aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
					aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
					aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
					aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
					aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);

					if (mesh->HasTangentsAndBitangents())
					{
						vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
						vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					meshSource->m_vertices.push_back(vertex);
				}

				// Indices
				for (size_t i = 0; i < mesh->mNumFaces; i++)
				{
					// we're using aiProcess_Triangulate so this should always be true
					Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
					meshSource->m_indices.push_back(index);

					meshSource->m_triangle_cache[m].emplace_back(meshSource->m_vertices[index.V1 + submesh.BaseVertex], meshSource->m_vertices[index.V2 + submesh.BaseVertex], meshSource->m_vertices[index.V3 + submesh.BaseVertex]);
				}
			}
            
            MeshNode& rootNode = meshSource->m_nodes.emplace_back();
			TraverseNodes(meshSource, scene->mRootNode, 0);

			for (const auto& submesh : meshSource->m_submeshes)
			{
				AABB transformedSubmeshAABB = submesh.BoundingBox;
				glm::vec3 min = glm::vec3(submesh.Transform * glm::vec4(transformedSubmeshAABB.Min, 1.0f));
				glm::vec3 max = glm::vec3(submesh.Transform * glm::vec4(transformedSubmeshAABB.Max, 1.0f));

				meshSource->m_bounding_box.Min.x = glm::min(meshSource->m_bounding_box.Min.x, min.x);
				meshSource->m_bounding_box.Min.y = glm::min(meshSource->m_bounding_box.Min.y, min.y);
				meshSource->m_bounding_box.Min.z = glm::min(meshSource->m_bounding_box.Min.z, min.z);
				meshSource->m_bounding_box.Max.x = glm::max(meshSource->m_bounding_box.Max.x, max.x);
				meshSource->m_bounding_box.Max.y = glm::max(meshSource->m_bounding_box.Max.y, max.y);
				meshSource->m_bounding_box.Max.z = glm::max(meshSource->m_bounding_box.Max.z, max.z);
			}
		}

        // TODO: 后续补充骨骼权重

        // Materials
        // Ref<Texture2D> defaultTexture = RenderSystem::GetInstance().GetDefaultWhiteTexture();
		if (scene->HasMaterials()) {
			meshSource->m_materials.reserve(scene->mNumMaterials);
			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
			{
				aiMaterial* aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();
				// PBR
				Ref<Material> material = Material::Create(RenderSystem::GetShaderLibrary()->Get("PBR_Static"), aiMaterialName.data);
				auto material_asset = Ref<MaterialAsset>::Create(material);

				aiString aiTexPath;
				glm::vec3 albedoColor(0.8f);
				float emission = 0.0f;
				aiColor3D aiColor, aiEmission;
				if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == AI_SUCCESS) {
					auto sRGBFromLinear = [](float value) {
						return value <= 0.0031308f ? 12.92f * value : 1.055f * glm::pow(value, 1.0f / 2.4f) - 0.055f;
					};
					albedoColor = { sRGBFromLinear(aiColor.r), sRGBFromLinear(aiColor.g), sRGBFromLinear(aiColor.b) };
				}

				if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, aiEmission) == AI_SUCCESS) {
					emission = aiEmission.r;
				}

				material_asset->SetAlbedoColor(albedoColor);
				material_asset->SetEmission(emission);

				float roughness, metalness;
				if (aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != AI_SUCCESS) {
					roughness = 0.4f;
				}
				if (aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metalness) != AI_SUCCESS) {
					metalness = 0.0f;
				}

				if (metalness < 0.9f)
					metalness = 0.0f;
				else
					metalness = 1.0f;

				material_asset->SetRoughness(roughness);
				material_asset->SetMetalness(metalness);

				LOG_INFO(fmt::format("Loaded material: {0}, Albedo: {1}, {2}, {3}, Roughness: {4}, Metalness: {5}, Emission: {6}",
					aiMaterialName.data, aiColor.r, aiColor.g, aiColor.b, roughness, metalness, emission
				));


			}
		}

        return nullptr;
    }


    void AssimpMeshImporter::TraverseNodes(Ref<MeshSource> meshSource, void* assimpNode, uint32_t nodeIndex, const glm::mat4& parentTransform) {
        // Implementation for traversing nodes
    }

}
