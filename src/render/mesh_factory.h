#pragma once

#include "render/mesh.h"

namespace Mint {
    // Need to change after implement asset
    class MeshFactory {
    public:
        static AssetHandle CreateCollisionCube(const glm::vec3& size);
        static AssetHandle CreateTextureCube(const glm::vec3& size);
        static Ref<Mesh> CreatePlane(const glm::vec3& size);
        static Ref<Mesh> CreateSphere(){}
        static Ref<Mesh> CreateCapsule() {}
        static Ref<Mesh> CreateOctahedron() {}

    };
}