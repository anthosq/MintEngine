#include "AssimpMeshImporter.h"
#include "asset/AssimpLogSystem.h"
#include "asset/asset.h"
#include "render/render_system.h"

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
        return nullptr;
    }


    void AssimpMeshImporter::TraverseNodes(Ref<MeshSource> meshSource, void* assimpNode, uint32_t nodeIndex, const glm::mat4& parentTransform) {
        // Implementation for traversing nodes
    }

}
