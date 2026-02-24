#include "render/mesh_factory.h"
#include <math.h>

namespace Mint {
    AssetHandle MeshFactory::CreateCollisionCube(const glm::vec3& size) {
        std::vector<Vertex> vertices;
        vertices.resize(8);
        vertices[0].Position = { -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f };
		vertices[1].Position = {  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f };
		vertices[2].Position = {  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f };
		vertices[3].Position = { -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f };
		vertices[4].Position = { -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f };
		vertices[5].Position = {  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f };
		vertices[6].Position = {  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f };
		vertices[7].Position = { -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f };

		vertices[0].Normal = { -1.0f, -1.0f,  1.0f };
		vertices[1].Normal = {  1.0f, -1.0f,  1.0f };
		vertices[2].Normal = {  1.0f,  1.0f,  1.0f };
		vertices[3].Normal = { -1.0f,  1.0f,  1.0f };
		vertices[4].Normal = { -1.0f, -1.0f, -1.0f };
		vertices[5].Normal = {  1.0f, -1.0f, -1.0f };
		vertices[6].Normal = {  1.0f,  1.0f, -1.0f };
		vertices[7].Normal = { -1.0f,  1.0f, -1.0f };

        std::vector<Index> indices;
        indices.resize(12);
		indices[0] =  { 0, 1, 2 };
		indices[1] =  { 2, 3, 0 };
		indices[2] =  { 1, 5, 6 };
		indices[3] =  { 6, 2, 1 };
		indices[4] =  { 7, 6, 5 };
		indices[5] =  { 5, 4, 7 };
		indices[6] =  { 4, 0, 3 };
		indices[7] =  { 3, 7, 4 };
		indices[8] =  { 4, 5, 1 };
		indices[9] =  { 1, 0, 4 };
		indices[10] = { 3, 2, 6 };
		indices[11] = { 6, 7, 3 };

        AssetHandle mesh_source = AssetManager::AddMemoryOnlyAsset(Ref<MeshSource>::Create(vertices, indices, glm::mat4(1.0f)));
        return AssetManager::AddMemoryOnlyAsset(Ref<StaticMesh>::Create(mesh_source));
    }

    AssetHandle MeshFactory::CreateTextureCube(const glm::vec3& size) {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;

        static float cube_vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
        };

        const int vertexCount = 36;
        vertices.resize(vertexCount);
        indices.resize(vertexCount / 3); // Though here it is just raw triangles, we'll map 1:1

        for (int i = 0; i < vertexCount; i++) {
            // Position stride = 8, offset 0
            vertices[i].Position = {
                cube_vertices[i * 8 + 0] * size.x,
                cube_vertices[i * 8 + 1] * size.y,
                cube_vertices[i * 8 + 2] * size.z
            };
            // Normal stride = 8, offset 3
            vertices[i].Normal = {
                cube_vertices[i * 8 + 3],
                cube_vertices[i * 8 + 4],
                cube_vertices[i * 8 + 5]
            };
            // TexCoords stride = 8, offset 6
            vertices[i].TexCoords = {
                cube_vertices[i * 8 + 6],
                cube_vertices[i * 8 + 7]
            };
        }

        // Generate simple indices since the raw data is already unwrapped triangles
        for (int i = 0; i < vertexCount / 3; i++) {
             indices[i] = { (unsigned int)(i * 3), (unsigned int)(i * 3 + 1), (unsigned int)(i * 3 + 2) };
        }

        AssetHandle sourceHandle = AssetManager::AddMemoryOnlyAsset(Ref<MeshSource>::Create(vertices, indices, glm::mat4(1.0f)));

        Ref<StaticMesh> staticMesh = Ref<StaticMesh>::Create(sourceHandle);
        return AssetManager::AddMemoryOnlyAsset(staticMesh);
    }

    Ref<Mesh> MeshFactory::CreatePlane(const glm::vec3& size) {
        std::vector<Vertex> vertices;
        vertices.resize(4);

        std::vector<Index> indices;
        indices.resize(2);

        // Placeholder implementation
        return nullptr;
    }
}