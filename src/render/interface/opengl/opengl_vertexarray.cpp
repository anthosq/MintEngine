#include "opengl_vertexarray.h"
#include "render/render_system.h"

namespace Mint {
    OpenGLVertexArray::OpenGLVertexArray() {
        RenderSystem::Submit([this]() {
            glCreateVertexArrays(1, &m_renderer_id);
        });
    }

    OpenGLVertexArray::~OpenGLVertexArray() {
        RenderSystem::Submit([=]() {
            glDeleteVertexArrays(1, &m_renderer_id);
        });
    }

    void OpenGLVertexArray::Bind() const {
        RenderSystem::Submit([this]() {
            glBindVertexArray(m_renderer_id);
        });
    }

    void OpenGLVertexArray::Unbind() const {
        RenderSystem::Submit([]() {
            glBindVertexArray(0);
        });
    }

    void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertex_buffer) {
        RenderSystem::Submit([this, vertex_buffer]() {
            glBindVertexArray(m_renderer_id);
            vertex_buffer->Bind();
            uint32_t index = 0;
            const BufferLayout& layout = vertex_buffer->GetLayout();
            for (const auto& element: layout) {
                glEnableVertexAttribArray(index);
            glVertexAttribPointer(index,
                element.GetComponentCount(),
                GetOpenGLDataType(element.type),
                element.normalized ? GL_TRUE : GL_FALSE,
                layout.GetStride(),
                (const void*)element.offset);
            index++;
            }
        });
        m_vertexBuffers.push_back(vertex_buffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& index_buffer) {
        RenderSystem::Submit([this, index_buffer]() {
            glBindVertexArray(m_renderer_id);
            index_buffer->Bind();
        });

        m_indexBuffer = index_buffer;
    }


}