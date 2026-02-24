#include "render/render_system.h"
#include "render/interface/opengl/opengl_renderer_api.h"


namespace Mint {

    // TODO: add render command queue for multithreading rendering
    // TODO: consider to use PIMPL pattern to hide implementation details for different Platforms
    // in case of different platform versions we don't need virtualization at all
    // just different cpp files for each platform with same methods,
    // we just need one level of "virtualization" and don't need to re-override those methods

    // From my experience when you need multiple/crossplatform implementation for some classes like renderer it is better to use pImpl idiom instead of virtual classes to nullify all overheads on function calls every frame. Also if you need virtualization always set final keyword on implementation class to allow compiler to optimize some things.

    // 注意, 参考Piccolo的RenderBuffer设计

    // 单例
    RenderSystem* RenderSystem::m_renderer = new RenderSystem();

    struct RenderData {
        Ref<ShaderLibrary> m_shader_library;
        Ref<Texture2D> white_texture;
    };

    static RenderData* m_renderData = nullptr;


    //temporary
    RenderSystem::SceneData* RenderSystem::m_sceneData = new SceneData;

    void RenderSystem::Init() {
        m_renderData = new RenderData();
        m_rendererAPI = std::make_shared<OpenGLRendererAPI>();
        m_renderData->m_shader_library = Ref<ShaderLibrary>::Create();
        RenderSystem::Submit([this]() {
            m_rendererAPI->Init();
        });

        // initialize renderdata members
        uint32_t white_texture_datas = 0xffffffff;
        m_renderData->white_texture = Texture2D::Create({.Format = TextureFormat::RGBA8, .Width = 1, .Height = 1}, Buffer(&white_texture_datas, sizeof(uint32_t)));
    }

    void RenderSystem::Shutdown() {
        RenderSystem::Submit([this]() {
            m_rendererAPI->Shutdown();
        });
        delete m_renderData;
    }

    void RenderSystem::Clear() {
        RenderSystem::Submit([this]() {
            m_rendererAPI->Clear();
        });
    }

    void RenderSystem::Clear(const glm::vec4& color) {
        RenderSystem::Submit([=]() {
            m_rendererAPI->Clear(color);
        });
    }

    void RenderSystem::DrawIndexed(uint32_t count, bool depth_test) {
        RenderSystem::Submit([=]() {
            m_rendererAPI->DrawIndexed(count, depth_test);
        });
    }

    void RenderSystem::DrawArrays(uint32_t mode, uint32_t count, uint32_t first, bool depth_test) {
        RenderSystem::Submit([=]() {
            m_rendererAPI->DrawArrays(mode, first, count, depth_test);
        });
    }

    // Maybe this should be called Flush?
    void RenderSystem::WaitAndRender() {
        m_renderer->m_commandQueue.Execute();
    }

    // temporary, modify after framebuffer is done?
    void RenderSystem::OnWindowResize(uint32_t width, uint32_t height) {
        RenderSystem::Submit([=](){
          glViewport(0, 0, width, height);
        });
    }

    Ref<ShaderLibrary> RenderSystem::GetShaderLibrary() {
        return m_renderData->m_shader_library;
    }

    Ref<Texture2D> RenderSystem::GetWhiteTexture() {
        return m_renderData->white_texture;
    }

    // !!TODO: Actually here we need scene camera which responsible for orthographic/perspective projection matrix
    // Temporary use EditorCamera

    void RenderSystem::BeginScene(EditorCamera& camera) {
        // Prepare something before rendering
        // environment map, cub map sample, camera...
        // projection view matrix, camera space, light

        m_sceneData->viewProjectionMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();

    }

    void RenderSystem::EndScene() {
        // End of rendering
    }

    // !!TODO: Need to set up UniformBuffer class to manage uniform buffer objects
    // Need to implement WaitAndExcute(), to support rendercommand queue
    // Need to move SetMat4 to uniform buffer instead of setting it one by one
    // move submit to render command, receiving lambda
    void RenderSystem::Submit(Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform, bool depth_test) {
        // not sure
        glm::mat4 viewProj = m_sceneData->viewProjectionMatrix;
        shader->Bind();
        shader->SetMat4("u_ViewProjection", viewProj);
        // Model matrix
        shader->SetMat4("u_Transform", transform);
        vertex_array->Bind();
        DrawIndexed(vertex_array->GetIndexBuffer()->GetCount(), depth_test);
        vertex_array->Unbind();
    }

    // temp?
    void RenderSystem::SubmitArrays(Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, uint32_t mode, uint32_t count, uint32_t first, const glm::mat4& transform, bool depth_test) {
        glm::mat4 viewProj = m_sceneData->viewProjectionMatrix;
        shader->Bind();
        shader->SetMat4("u_ViewProjection", viewProj);
        // Model matrix
        shader->SetMat4("u_Transform", transform);
        vertex_array->Bind();
        DrawArrays(mode, count, first, depth_test);
        vertex_array->Unbind();
    }
}
