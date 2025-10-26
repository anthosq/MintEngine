#include "render/render_system.h"


namespace Mint {

    // TODO: consider to use PIMPL pattern to hide implementation details for different Platforms
    // in case of different platform versions we don't need virtualization at all
    // just different cpp files for each platform with same methods,
    // we just need one level of "virtualization" and don't need to re-override those methods

    // From my experience when you need multiple/crossplatform implementation for some classes like renderer it is better to use pImpl idiom instead of virtual classes to nullify all overheads on function calls every frame. Also if you need virtualization always set final keyword on implementation class to allow compiler to optimize some things.

    // 注意, 参考Piccolo的RenderBuffer设计

    //temporary
    RenderSystem::SceneData* RenderSystem::m_sceneData = new SceneData;

    void RenderSystem::Init() {
        // Initialize RenderAPI
        RenderCommand::Init();
    }

    void RenderSystem::BeginScene(Camera& camera) {
        // Prepare something before rendering
        // environment map, cub map sample, camera...
        // projection view matrix, camera space, light

        m_sceneData->viewProjectionMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();

    }

    void RenderSystem::EndScene() {
        // End of rendering
    }

    void RenderSystem::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform) {
        // not sure
        shader->Bind();
        shader->SetMat4("u_ViewProjection", m_sceneData->viewProjectionMatrix);
        // Model matrix
        shader->SetMat4("u_Transform", transform);
        vertex_array->Bind();
        RenderCommand::DrawIndexed(vertex_array);
    }
}
