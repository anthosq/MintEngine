#include "render/interface/opengl/opengl_framebuffer.h"
#include "render/render_system.h"

namespace Mint {
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec) {
    }


    OpenGLFramebuffer::~OpenGLFramebuffer() {
        Release();
    }

    void OpenGLFramebuffer::Bind() const {
        RenderSystem::Submit([this]() {
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        });
    }

    void OpenGLFramebuffer::Unbind() const {
        RenderSystem::Submit([this]() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        });
    }

    void OpenGLFramebuffer::BindTexture(uint32_t attachmentIndex, uint32_t slot) const {
        RenderSystem::Submit([this, attachmentIndex, slot]() {
            glBindTextureUnit(slot, m_ColorAttachments[attachmentIndex]);
        });
    }

    

    void OpenGLFramebuffer::Release() {
        // Release the framebuffer resources
    }

    void OpenGLFramebuffer::Invalidate() {
        Ref<OpenGLFramebuffer> framebuffer = this;
        RenderSystem::Submit([framebuffer]() mutable {
            // Invalidate the framebuffer on the render thread
            framebuffer->RenderThread_Invalidate();
        });
    }

    void OpenGLFramebuffer::RenderThread_Invalidate() {

    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
        m_Width = width;
        m_Height = height;

        for (auto& callback : m_ResizeCallbacks) {
            callback(Ref<Framebuffer>(this));
        }
    }

    void OpenGLFramebuffer::AddResizeCallback(std::function<void(Ref<Framebuffer>)> const& callback) {
        m_ResizeCallbacks.push_back(callback);
    }

    int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) const {
        int pixelData = 0;
        // Read the pixel data from the framebuffer
        return pixelData;
    }

}