#include "render/mesh.h"


#include "log_system.h"
#include "render/interface/opengl/gl_common.h"

namespace Mint {
    // temporary implementation using Assimp

    // temporary anonymous namespace for internal linkage
    namespace {
        const unsigned int ImportFlags =
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_SortByPType |
            aiProcess_PreTransformVertices |
            aiProcess_GenNormals |
            aiProcess_GenUVCoords |
            aiProcess_OptimizeMeshes |
            aiProcess_Debone |
            aiProcess_ValidateDataStructure;
    }

    struct LogStream : public Assimp::LogStream
    {
        static void Initialize() {
            if (Assimp::DefaultLogger::isNullLogger()) {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
            }
        }

        void write(const char* message) override {
            LOG_ERROR(fmt::format("Assimp error: {0}", message));
        }
    };

}

