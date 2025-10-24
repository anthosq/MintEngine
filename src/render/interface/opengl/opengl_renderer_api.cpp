#include "opengl_renderer_api.h"

namespace Mint {
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
        vertexArray->Bind();
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }
}