#include "opengl_renderer_api.h"

namespace Mint {
    void OpenGLRendererAPI::Init() {
        // Enable depth testing
        // glEnable(GL_DEPTH_TEST);
        // glDepthFunc(GL_LESS);

        // Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable face culling
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
    }

    void OpenGLRendererAPI::Clear() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void OpenGLRendererAPI::ClearDepth() {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }
 
    void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray) {
        // 让下一个绘制覆盖VAO
        // 需要重构以支持skybox
        vertexArray->Bind();
        auto indexbuffer = vertexArray->GetIndexBuffer();
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }
}