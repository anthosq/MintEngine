#include "opengl_vertexarray.h"

namespace Mint {
    OpenGLVertexArray::OpenGLVertexArray() {
        glCreateVertexArrays(1, &m_renderer_id);
    }

    OpenGLVertexArray::~OpenGLVertexArray() {
        glDeleteVertexArrays(1, &m_renderer_id);
    }

    void OpenGLVertexArray::Bind() const {
        glBindVertexArray(m_renderer_id);
    }

    void OpenGLVertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer) {
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
        m_vertexBuffers.push_back(vertex_buffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer) {

        glBindVertexArray(m_renderer_id);
        index_buffer->Bind();

        m_indexBuffer = index_buffer;
    }


}