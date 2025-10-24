#include "shader.h"
#include "render/interface/opengl/opengl_shader.h"
#include "render/renderer_api.h"

namespace Mint {
    Shader* Shader::Create(const std::string &vertex_src, const std::string &fragment_src) {
        switch (RendererAPI::GetAPI()) {
            case RendererAPI::API::None:            return nullptr;
            case RendererAPI::API::OpenGL:          return new OpenGLShader(vertex_src, fragment_src);
        }

        return nullptr;
    }
}