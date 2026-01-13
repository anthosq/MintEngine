#pragma once

#include "core/ref.h"
#include <cstdint>


namespace Mint {
    using RendererID = uint32_t;

    // 
    struct FramebufferSpecification {
        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool SwapChainTarget = false;
    };

    class Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void BindTexture(uint32_t slot = 0) const = 0;

        virtual RendererID GetRendererID() const = 0;
        virtual RendererID GetColorAttachmentRendererID() const = 0;
        virtual RendererID GetDepthAttachmentRendererID() const = 0;

        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}