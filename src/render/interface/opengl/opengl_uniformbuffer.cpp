#include "render/interface/opengl/opengl_uniformbuffer.h"
#include "render/render_system.h"
#include "render/interface/opengl/gl_common.h"


namespace Mint {
    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding) {
        m_Binding = binding;
        m_Size = size;
        // m_local_storage.Allocate(size);

        RenderSystem::Submit([this, size, binding]() mutable {
            glCreateBuffers(1, &m_RendererID);
            glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
        });
    }

    OpenGLUniformBuffer::~OpenGLUniformBuffer() {
        // m_local_storage.Release();

        RenderSystem::Submit([this]() mutable {
            glDeleteBuffers(1, &m_RendererID);
        });
    }

    void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) {
        // Not sure why we need a local storage here
        // Why not create a Snapshot
        // m_local_storage.Copy(data, size);
        Buffer temp_buffer = Buffer::Copy(data, size);
        RenderSystem::Submit([this, size, offset, temp_buffer]() mutable {
            this->RenderThread_SetData(temp_buffer.Data, size, offset);
        });
    }

    void OpenGLUniformBuffer::RenderThread_SetData(const void* data, uint32_t size, uint32_t offset) {
        glNamedBufferSubData(m_RendererID, offset, size, data);
    }

}