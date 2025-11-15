#include "render/vertex_array.h"
#include "render/interface/opengl/opengl_vertexarray.h"
#include "render/render_system.h"


namespace Mint {
    Ref<VertexArray> VertexArray::Create() {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::None:
                return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLVertexArray>::Create();
            default:
                return nullptr;
        }
    }

    
}