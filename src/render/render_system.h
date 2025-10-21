#pragma once
#include "render_command.h"
#include "render/camera.h"
#include "render/shader.h"

namespace Mint {

    class RenderSystem {
    public:
        static void BeginScene(Camera& camera);

        static void EndScene();

        static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertex_array);

        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    private:

        struct SceneData {
            glm::mat4 viewProjectionMatrix;
        };
        static SceneData* m_sceneData;
    };
}