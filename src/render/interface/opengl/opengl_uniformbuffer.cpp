#include "render/interface/opengl/opengl_uniformbuffer.h"


namespace Mint {

    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
        : m_Size(size), m_BindingPoint(binding) {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    OpenGLUniformBuffer::~OpenGLUniformBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLUniformBuffer::Bind() const {
        glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
    }

    void OpenGLUniformBuffer::Unbind() const {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

}