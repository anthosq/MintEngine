#include "uniform_buffer.h"
#include "render/interface/opengl/opengl_uniformbuffer.h"
#include "render/render_system.h"
#include "core/ref.h"

namespace Mint {

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLUniformBuffer>::Create(size, binding);
            case RendererAPI::RenderAPIType::None:
                return nullptr;
        }
        return nullptr;
    }
}
