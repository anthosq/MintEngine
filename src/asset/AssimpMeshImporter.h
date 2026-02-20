#pragma once

#include <filesystem>
#include <glm/glm.hpp>

#include "render/mesh.h"

namespace Mint {
    class AssimpMeshImporter {
    public:
        AssimpMeshImporter(const std::filesystem::path& path);
        Ref<MeshSource> ImportToMeshSource();

    private:
        void TraverseNodes(Ref<MeshSource> meshSource, void* assimpNode, uint32_t nodeIndex, const glm::mat4& parentTransform = glm::mat4(1.0f));
    private:
        const std::filesystem::path m_Path;
    };
}