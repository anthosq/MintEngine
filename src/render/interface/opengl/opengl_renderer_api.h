#pragma once
#include "render/renderer_api.h"

namespace Mint {
    class OpenGLRendererAPI : public RendererAPI {
    public:
        virtual void Init() override;

        virtual void Clear() override;
        virtual void Clear(const glm::vec4& color) override;

        virtual void DrawIndexed(unsigned int count, bool depthTest = true) override;
    };
}