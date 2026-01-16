#include "render/framebuffer.h"
#include "render/interface/opengl/opengl_framebuffer.h"

#include "render/render_system.h"


namespace Mint {
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLFramebuffer>::Create(spec);
            case RendererAPI::RenderAPIType::None:
                return nullptr;
        }
        return nullptr;
    }
}
