#pragma once
#include "render/uniform_buffer.h"

namespace Mint {
    class OpenGLUniformBuffer : public UniformBuffer {
    public:
        OpenGLUniformBuffer(uint32_t size);
        virtual ~OpenGLUniformBuffer() {};

        virtual void SetData(const void* data, uint32_t size, uint32_t offset) override;
        virtual void RenderThread_SetData(const void* data, uint32_t size, uint32_t offset) override;

    private:
        uint32_t m_RendererID;
    };
}