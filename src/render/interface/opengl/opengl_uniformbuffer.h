#pragma once
#include "render/uniform_buffer.h"
#include "core/buffer.h"

namespace Mint {
    class OpenGLUniformBuffer : public UniformBuffer {
    public:
        OpenGLUniformBuffer(uint32_t size, uint32_t binding);
        virtual ~OpenGLUniformBuffer();

        virtual void SetData(const void* data, uint32_t size, uint32_t offset) override;
        virtual void RenderThread_SetData(const void* data, uint32_t size, uint32_t offset) override;

        // Get Method
        uint32_t GetRenderID() const { return m_RendererID; };
        uint32_t GetSize() const { return m_Size; };
        uint32_t GetBinding() const { return m_Binding; };

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_Size = 0;
        uint32_t m_Binding = 0;
    };
}