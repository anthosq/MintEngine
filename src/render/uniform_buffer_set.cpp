#include "render/uniform_buffer_set.h"
#include "render/render_system.h"


namespace Mint {

    Ref<UniformBufferSet> UniformBufferSet::Create() {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<UniformBufferSet>::Create();
            case RendererAPI::RenderAPIType::None:
                return nullptr;
        }
        return nullptr;
    }
    
}