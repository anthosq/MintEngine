#pragma once
#include "render/renderer_api.h"

namespace Mint {
    class OpenGLRendererAPI : public RendererAPI {
    public:
        virtual void Init() override;
        virtual void Shutdown() override {};

        virtual void Clear() override;
        virtual void Clear(const glm::vec4& color) override;

        virtual void DrawIndexed(uint32_t count, bool depthTest = true) override;
        virtual void DrawArrays(uint32_t mode, uint32_t first, uint32_t count, bool depthTest = true) override;
    };
}