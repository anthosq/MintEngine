#include "render/render_system.h"
#include "render/interface/opengl/opengl_shader.h"


namespace Mint {

    // TODO: consider to use PIMPL pattern to hide implementation details for different Platforms
    // in case of different platform versions we don't need virtualization at all
    // just different cpp files for each platform with same methods,
    // we just need one level of "virtualization" and don't need to re-override those methods

    // From my experience when you need multiple/crossplatform implementation for some classes like renderer it is better to use pImpl idiom instead of virtual classes to nullify all overheads on function calls every frame. Also if you need virtualization always set final keyword on implementation class to allow compiler to optimize some things.

    // 注意, 参考Piccolo的RenderBuffer设计

    void RenderSystem::BeginScene() {
        // Prepare something before rendering
        // environment map, cub map sample, camera...
        // projection view matrix, camera space, light

    }

    void RenderSystem::EndScene() {
        // End of rendering
    }

    void RenderSystem::Submit(const std::shared_ptr<VertexArray>& vertex_array) {
        // not sure
        vertex_array->Bind();
        RenderCommand::DrawIndexed(vertex_array);
    }
}
