#include "render/renderer2D.h"

namespace Mint {
    // 之后再说

    void Renderer2D::Init() {}
    void Renderer2D::Shutdown() {}

    // 需要实现OthographicCamera
    void Renderer2D::BeginScene(const glm::mat4 &viewProjectionMatrix) {}

    void Renderer2D::EndScene() {}

    // Primitives
    void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color) {}
    void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color) {}

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color) {}
    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color) {}
}