#pragma once
#include <glm/glm.hpp>
#include "render/vertex_array.h"

namespace Mint {
    class RendererAPI {
        public:
            enum class RenderAPIType
            {
                None = 0,
                OpenGL = 1,
            };
        public:

            virtual void Init() = 0;

            virtual void Clear() = 0;

            virtual void Clear(const glm::vec4& color) = 0;

            // !TODO: change to accept count and depth test flag
            virtual void DrawIndexed(unsigned int count, bool depthTest = true) = 0;

            inline static RenderAPIType GetAPIType() { return s_api_type; }
        private:
            inline static RenderAPIType s_api_type = RendererAPI::RenderAPIType::OpenGL;
    };
}