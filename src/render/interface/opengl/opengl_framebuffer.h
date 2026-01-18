#pragma once

#include "render/framebuffer.h"

namespace Mint {
    class OpenGLFramebuffer : public Framebuffer {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        virtual void Resize(uint32_t width, uint32_t height) override;
        virtual void AddResizeCallback(const std::function<void(Ref<Framebuffer>)>& callback) override;

        void Invalidate();
        void RenderThread_Invalidate();

        void Release();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void BindTexture(uint32_t attachmentIndex, uint32_t slot = 0) const override;

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }

        virtual uint32_t GetRendererID() const override { return m_RendererID; }

        // uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const;
        // uint32_t GetDepthAttachmentRendererID() const { return m_DepthAttachment; }

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

        int ReadPixel(uint32_t attachmentIndex, int x, int y) const;


    private:
        uint32_t m_RendererID = 0;
        FramebufferSpecification m_Specification;
        uint32_t m_Width = 0, m_Height = 0;

        // std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
        // FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t m_DepthAttachment = 0;

        std::vector<std::function<void(Ref<Framebuffer>)>> m_ResizeCallbacks;
    };
}