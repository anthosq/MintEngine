#pragma once

#include "core/ref.h"
#include <cstdint>
#include <functional>
#include <vector>


namespace Mint {
    // 完成Material后来处理

    // 后续移动到Image相关内容
    enum class FramebufferTextureFormat {
        None = 0,
        RGBA8,
        RGBA16F,
        RED_INTEGER,
        // Depth/Stencil
        DEPTH24STENCIL8,
    };

    // FramebufferBlendMode?

    struct FramebufferTextureSpecification {

        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format) : Format(format) {}

        FramebufferTextureFormat Format;
        // bool Blend = false;
        // FramebufferBlendMode Blend = FramebufferBlendMode::None;
    };

    struct FramebufferAttachmentSpecification {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) : Attachments(attachments) {}

        std::vector<FramebufferTextureSpecification> Attachments;
    };


    struct FramebufferSpecification {
        uint32_t Width = 1280;
        uint32_t Height = 720;

        // for MSAA
        uint32_t Samples = 1;

        FramebufferAttachmentSpecification Attachments;

        bool SwapChainTarget = false;
    };

    class Framebuffer : public RefCounter {
    public:
        virtual ~Framebuffer() = default;

        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual void AddResizeCallback(const std::function<void(Ref<Framebuffer>)>& callback) = 0;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void BindTexture(uint32_t attachmentIndex, uint32_t slot = 0) const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual uint32_t GetRendererID() const = 0;
        virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
        virtual uint32_t GetDepthAttachmentRendererID() const = 0;

        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}