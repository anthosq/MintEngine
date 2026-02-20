#pragma once
#include <cstdint>
#include "render/buffer.h"
#include "render/interface/opengl/gl_common.h"
#include "core/buffer.h"

namespace Mint {
    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(uint32_t size);
        OpenGLVertexBuffer(void* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual const BufferLayout& GetLayout() const override { return m_layout; }
        virtual void SetLayout(const BufferLayout& layout) override { m_layout = layout; }

        virtual void SetData(const void* data, uint32_t size) override;

        void Bind() const override;
        void Unbind() const override;

    private:
        uint32_t m_renderer_id;
        uint32_t m_size;
        BufferLayout m_layout;
        

        Buffer m_local_data;
    };

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(void* indices, uint32_t count);
        virtual ~OpenGLIndexBuffer();

        void Bind() const;
        void Unbind() const;

        uint32_t GetCount() const { return m_count; }
        void SetData(const void* data, uint32_t size) override;

    private:
        uint32_t m_renderer_id;
        uint32_t m_count;
        uint32_t m_size;

        Buffer m_local_data;
    };
}