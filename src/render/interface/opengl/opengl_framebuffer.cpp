#include "render/interface/opengl/opengl_framebuffer.h"
#include "render/render_system.h"
#include "render/interface/opengl/opengl_utils.h"
#include "render/interface/opengl/gl_common.h"
#include "log_system.h"

namespace Mint {



    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec) {
            for (auto spec: m_Specification.Attachments.Attachments) {
                if (!Utils::IsDepthFormat(spec.Format)) {
                    m_ColorAttachmentSpecs.emplace_back(spec);
                } else {
                    m_DepthAttachmentSpec = spec;
                }
            }

            m_Specification.Width = spec.Width;
            m_Specification.Height = spec.Height;
            Invalidate();
    }


    OpenGLFramebuffer::~OpenGLFramebuffer() {
        Release();
    }

    void OpenGLFramebuffer::Bind() const {
        RenderSystem::Submit([this]() {
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
            glViewport(0, 0, m_Specification.Width,  m_Specification.Height);
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
        RenderSystem::Submit([this]() {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);
        });
    }

    void OpenGLFramebuffer::Invalidate() {
        Ref<OpenGLFramebuffer> framebuffer = this;
        RenderSystem::Submit([framebuffer]() mutable {
            // Invalidate the framebuffer on the render thread
            framebuffer->RenderThread_Invalidate();
        });
    }

    void OpenGLFramebuffer::RenderThread_Invalidate() {
        if (m_RendererID) {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);

            m_ColorAttachments.clear();
            m_DepthAttachment = 0;
        }

        // Create framebuffer
        glCreateFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        bool multisample = m_Specification.Samples > 1;

        // 需要调整
        if (m_ColorAttachmentSpecs.size() > 0) {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            Utils::CreateTextures(multisample, m_ColorAttachments.data(), (uint32_t)m_ColorAttachments.size());

            for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
                Utils::BindTexture(multisample, m_ColorAttachments[i]);
                switch (m_ColorAttachmentSpecs[i].Format) {
                    case FramebufferTextureFormat::RGBA8: {
                        Utils::AttachColorTexture(
                            m_ColorAttachments[i],
                            m_Specification.Samples,
                            GL_RGBA8,
                            GL_RGBA,
                            m_Specification.Width,
                            m_Specification.Height,
                            (int)i
                        );
                        break;
                    }
                    case FramebufferTextureFormat::RGBA16F: {
                        Utils::AttachColorTexture(
                            m_ColorAttachments[i],
                            m_Specification.Samples,
                            GL_RGBA16F,
                            GL_RGBA,
                            m_Specification.Width,
                            m_Specification.Height,
                            (int)i
                        );
                        break;
                    }
                    case FramebufferTextureFormat::RED_INTEGER: {
                        Utils::AttachColorTexture(
                            m_ColorAttachments[i],
                            m_Specification.Samples,
                            GL_R32I,
                            GL_RED_INTEGER,
                            m_Specification.Width,
                            m_Specification.Height,
                            (int)i
                        );
                        break;
                    }
                }
            }
        }

        if (m_DepthAttachmentSpec.Format != FramebufferTextureFormat::None) {
            // Create depth attachment
            Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
            Utils::BindTexture(multisample, m_DepthAttachment);
            switch (m_DepthAttachmentSpec.Format) {
                case FramebufferTextureFormat::DEPTH24STENCIL8: {
                    Utils::AttachDepthTexture(
                        m_DepthAttachment,
                        m_Specification.Samples,
                        GL_DEPTH24_STENCIL8,
                        GL_DEPTH_STENCIL_ATTACHMENT,
                        m_Specification.Width,
                        m_Specification.Height
                    );
                    break;
                }
            }
        }

        if (m_ColorAttachments.size() > 1) {
            GLenum buffers[16] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                                  GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7,
                                  GL_COLOR_ATTACHMENT8, GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
                                  GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15 };
            glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
        } else if (m_ColorAttachments.empty()) {
            // Only depth-pass
            glDrawBuffer(GL_NONE);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer is not complete");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
        if (m_Specification.Width == width && m_Specification.Height == height)
            return;
        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();

        // for (auto& callback : m_ResizeCallbacks) {
        //     callback(Ref<Framebuffer>(this));
        // }
    }

    uint32_t OpenGLFramebuffer::GetColorAttachmentRendererID(uint32_t index) const {
        if (index >= m_ColorAttachments.size()) {
            LOG_ERROR("Invalid color attachment index");
            return 0;
        }
        return m_ColorAttachments[index];
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