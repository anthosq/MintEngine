#include "buffer.h"
#include "render_system.h"
#include "render/interface/opengl/opengl_buffer.h"

namespace Mint {
    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
        // 这里后续可以根据RHI的不同, 创建不同的实现类
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::None:
                return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLVertexBuffer>::Create(vertices, size);
            default:
                return nullptr;
        }
    }

    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::None:
                return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:
                return  Ref<OpenGLVertexBuffer>::Create(size);
            default:
                return nullptr;
        } 
    }


    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::None:
                return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLIndexBuffer>::Create(indices, size);
            default:
                return nullptr;
        }
    }

}