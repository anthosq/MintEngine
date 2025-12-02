#include "opengl_buffer.h"
#include "render/render_system.h"


namespace Mint {
    // 未来加入Buffer类后需要调整
    // VertexBuffer
    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) {
        RenderSystem::Submit([this, size] () {
        glCreateBuffers(1, &m_renderer_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        // glNamedBufferData(m_renderer_id, size, nullptr, GL_DYNAMIC_DRAW);
    });
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
        RenderSystem::Submit([this, vertices, size] () {
            glCreateBuffers(1, &m_renderer_id);
            glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
            glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        });
        // glNamedBufferData(m_renderer_id, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        RenderSystem::Submit([=] () {
            glDeleteBuffers(1, &m_renderer_id);
        });
    }

    void OpenGLVertexBuffer::SetData(const void* data, uint32_t size) {
        RenderSystem::Submit([=] () {
            glNamedBufferSubData(m_renderer_id, 0, size, data);
        });
    }

    void OpenGLVertexBuffer::Bind() const {
        RenderSystem::Submit([this] () {
            glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        });
    }

    void OpenGLVertexBuffer::Unbind() const {
        RenderSystem::Submit([=] () {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        });
    }

    // IndexBuffer
    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t *indices, uint32_t count) : m_count(count) {
        RenderSystem::Submit([this, indices, count] () {
            glCreateBuffers(1, &m_renderer_id);
            // GL_ELEMENT_ARRAY_BUFFER只有在绑定VAO时才有效, 所以这里用GL_ARRAY_BUFFER
            // 这样可以在不绑定VAO的情况下使用索引缓冲
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        });

    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        RenderSystem::Submit([=] () {
            glDeleteBuffers(1, &m_renderer_id);
        });
    }

    void OpenGLIndexBuffer::Bind() const {
        RenderSystem::Submit([this] () {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
        });
    }

    void OpenGLIndexBuffer::Unbind() const {
        RenderSystem::Submit([=] () {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        });
    }

    void OpenGLIndexBuffer::SetData(const void* data, uint32_t size) {
        RenderSystem::Submit([=] () {
            glNamedBufferSubData(m_renderer_id, 0, size, data);
        });
    }

}