#pragma once
#include "render/vertex_array.h"



namespace Mint {
    class OpenGLVertexArray : public VertexArray {
        public:
            OpenGLVertexArray();
            virtual ~OpenGLVertexArray();

            virtual void Bind() const override;
            virtual void Unbind() const override;

            virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer) override;
            virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer) override;

            virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_vertexBuffers; }
            virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_indexBuffer; }

        private:
            uint32_t m_renderer_id;
            uint32_t m_vertex_buffer_index = 0;
            std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
            std::shared_ptr<IndexBuffer> m_indexBuffer;
    };
}