#pragma once
#include "render/renderer_api.h"

namespace Mint {
    class RenderCommand {
    public:
        inline static void Init() {
            s_renderer_api->Init();
        }

        inline static void Clear() {
            s_renderer_api->Clear();
        }

        inline static void ClearDepth() {
            s_renderer_api->ClearDepth();
        }

        inline static void SetClearColor(const glm::vec4& color) {
            s_renderer_api->SetClearColor(color);
        }

        inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
            s_renderer_api->DrawIndexed(vertexArray);
        }

    private:
        static RendererAPI* s_renderer_api;
    };
}