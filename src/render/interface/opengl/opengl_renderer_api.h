#pragma once
#include "render/renderer_api.h"

namespace Mint {
    class OpenGLRendererAPI : public RendererAPI {
    public:
        virtual void Clear() override;
        virtual void ClearDepth() override;
        virtual void SetClearColor(const glm::vec4& color) override;

        virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
    };
}