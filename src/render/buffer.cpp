#include "buffer.h"
#include "render_system.h"
#include "render/interface/opengl/opengl_buffer.h"

namespace Mint {    
    VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
        // 这里后续可以根据RHI的不同, 创建不同的实现类
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::API::None:
                return nullptr;
            case RendererAPI::API::OpenGL:
                return new OpenGLVertexBuffer(vertices, size);
            default:
                return nullptr;
        }
    }

    VertexBuffer* VertexBuffer::Create(uint32_t size) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::API::None:
                return nullptr;
            case RendererAPI::API::OpenGL:
                return new OpenGLVertexBuffer(size);
            default:
                return nullptr;
        } 
    }


    IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::API::None:
                return nullptr;
            case RendererAPI::API::OpenGL:
                return new OpenGLIndexBuffer(indices, count);
            default:
                return nullptr;
        }
    }

}