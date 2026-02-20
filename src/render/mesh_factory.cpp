#include "render/mesh_factory.h"
#include <math.h>

namespace Mint {
    Ref<Mesh> MeshFactory::CreateCube(const glm::vec3& size) {
        std::vector<Vertex> vertices;
        vertices.resize(8);


        std::vector<Index> indices;
        indices.resize(12);

        // Placeholder implementation
        return nullptr;
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