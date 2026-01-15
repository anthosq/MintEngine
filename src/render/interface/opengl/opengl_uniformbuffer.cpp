#include "render/interface/opengl/opengl_uniformbuffer.h"
#include "render/render_system.h"
#include "render/interface/opengl/gl_common.h"


namespace Mint {
    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size) {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
    }

    void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
    }

    void OpenGLUniformBuffer::RenderThread_SetData(const void* data, uint32_t size, uint32_t offset)
    {
        // Set Data in Render Thread?
    }

}