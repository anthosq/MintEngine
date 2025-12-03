#include "opengl_buffer.h"
#include "render/render_system.h"


namespace Mint {
    // 未来加入Buffer类后需要调整
    // VertexBuffer
    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) : m_size(size) {
        RenderSystem::Submit([this] () {
        glCreateBuffers(1, &m_renderer_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        glBufferData(GL_ARRAY_BUFFER, m_size, nullptr, GL_DYNAMIC_DRAW);
        // glNamedBufferData(m_renderer_id, size, nullptr, GL_DYNAMIC_DRAW);
    });
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) : m_size(size) {
        m_local_data = Buffer::Copy((byte*)vertices, size);
        RenderSystem::Submit([=] () {
            glCreateBuffers(1, &m_renderer_id);
            glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
            glBufferData(GL_ARRAY_BUFFER, m_size, m_local_data.Data, GL_STATIC_DRAW);
        });
        // glNamedBufferData(m_renderer_id, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        RenderSystem::Submit([this] () {
            glDeleteBuffers(1, &m_renderer_id);
        });
    }

    // offset?
    void OpenGLVertexBuffer::SetData(const void* data, uint32_t size) {
        m_local_data = Buffer::Copy((byte*)data, size);
        m_size = size;
        RenderSystem::Submit([this] () {
            glNamedBufferSubData(m_renderer_id, 0, m_size, m_local_data.Data);
        });
    }

    void OpenGLVertexBuffer::Bind() const {
        RenderSystem::Submit([this] () {
            glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        });
    }

    void OpenGLVertexBuffer::Unbind() const {
        RenderSystem::Submit([this] () {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        });
    }

    // IndexBuffer
    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t *indices, uint32_t size) : m_size(size) {
        m_local_data = Buffer::Copy((byte*)indices, size);
        // 临时
        m_count = size / sizeof(uint32_t);
        RenderSystem::Submit([this] () {
            glCreateBuffers(1, &m_renderer_id);
            // GL_ELEMENT_ARRAY_BUFFER只有在绑定VAO时才有效, 所以这里用GL_ARRAY_BUFFER
            // 这样可以在不绑定VAO的情况下使用索引缓冲
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(uint32_t), m_local_data.Data, GL_STATIC_DRAW);
        });

    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        RenderSystem::Submit([this] () {
            glDeleteBuffers(1, &m_renderer_id);
        });
    }

    void OpenGLIndexBuffer::Bind() const {
        RenderSystem::Submit([this] () {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
        });
    }

    void OpenGLIndexBuffer::Unbind() const {
        RenderSystem::Submit([this] () {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        });
    }

    void OpenGLIndexBuffer::SetData(const void* data, uint32_t size) {
        m_local_data = Buffer::Copy((byte*)data, size);
        m_size = size;
        RenderSystem::Submit([this] () {
            glNamedBufferSubData(m_renderer_id, 0, m_size, m_local_data.Data);
        });
    }

}