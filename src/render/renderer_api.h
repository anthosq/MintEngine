#pragma once
#include <glm/glm.hpp>
#include "vertex_array.h"

namespace Mint {
    class RendererAPI {
        public:
            enum class API
            {
                None = 0,
                OpenGL = 1,
            };
        public:
            virtual void Clear() = 0;
            virtual void ClearDepth() = 0;

            virtual void SetClearColor(const glm::vec4& color) = 0;

            virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;
        
            inline static API GetAPI() { return s_api; }
        private:
            static API s_api;
    };
}