#pragma once
#include "render/uniform_buffer.h"

namespace Mint {
    class OpenGLUniformBuffer : public UniformBuffer {
    public:
        OpenGLUniformBuffer(uint32_t size, uint32_t binding);
        virtual ~OpenGLUniformBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

    private:
        uint32_t m_RendererID;
        uint32_t m_Size;
        uint32_t m_BindingPoint;
    };
}