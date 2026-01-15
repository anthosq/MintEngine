#include "uniform_buffer.h"
#include "render/interface/opengl/opengl_uniformbuffer.h"
#include "render/render_system.h"
#include "core/ref.h"

namespace Mint {

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLUniformBuffer>::Create(size);
            case RendererAPI::RenderAPIType::None:
                return nullptr;
        }
        return nullptr;
    }
}
