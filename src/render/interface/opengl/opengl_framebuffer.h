#include "render/framebuffer.h"

namespace Mint {
    class OpenGLFramebuffer : public Framebuffer {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer() {};      
    
    private:
        FramebufferSpecification m_Specification;
        RendererID m_RendererID;
        RendererID m_ColorAttachment;
        RendererID m_DepthAttachment;
    };
}