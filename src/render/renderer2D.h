#pragma once

#include "render/render_command_queue.h"
#include "render/texture.h"
#include "render/uniform_buffer_set.h"

namespace Mint {
    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        // 需要实现OthographicCamera
        static void BeginScene(const glm::mat4& viewProjectionMatrix);

        static void EndScene();

        // Primitives
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
    };
}