#include "render/vertex_array.h"
#include "render/interface/opengl/opengl_vertexarray.h"
#include "render/render_system.h"


namespace Mint {
    VertexArray* VertexArray::Create() {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::API::None:
                return nullptr;
            case RendererAPI::API::OpenGL:
                return new OpenGLVertexArray();
            default:
                return nullptr;
        }
    }

    
}