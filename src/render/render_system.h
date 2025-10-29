#pragma once
#include "render/render_command.h"
#include "render/camera.h"
#include "render/shader.h"
#include "Core.h"

namespace Mint {

    class RenderSystem {
    public:
        static void Init();

        static void BeginScene(Camera& camera);

        static void EndScene();

        // temporary adding Transform
        static void Submit(Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform = glm::mat4(1.0f));

        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    private:

        struct SceneData {
            glm::mat4 viewProjectionMatrix;
        };
        static SceneData* m_sceneData;
    };
}