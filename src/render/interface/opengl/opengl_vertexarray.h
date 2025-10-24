#pragma once
#include "render/vertex_array.h"



namespace Mint {
    class OpenGLVertexArray : public VertexArray {
        public:
            OpenGLVertexArray();
            virtual ~OpenGLVertexArray();

            virtual void Bind() const override;
            virtual void Unbind() const override;

            virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertex_buffer) override;
            virtual void SetIndexBuffer(const Ref<IndexBuffer>& index_buffer) override;

            virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_vertexBuffers; }
            virtual const Ref<IndexBuffer>& GetIndexBuffer() const { return m_indexBuffer; }

        private:
            uint32_t m_renderer_id;
            uint32_t m_vertex_buffer_index = 0;
            std::vector<Ref<VertexBuffer>> m_vertexBuffers;
            Ref<IndexBuffer> m_indexBuffer;
    };
}