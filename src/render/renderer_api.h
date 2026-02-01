#pragma once
#include <glm/glm.hpp>
#include "render/vertex_array.h"

namespace Mint {
    struct RenderAPICapabilities
    {
        std::string Vendor;
        std::string Renderer;
        std::string Version;

        int MaxSamples;
        float MaxAnisotropy;
    };

    class RendererAPI {
        public:
            enum class RenderAPIType
            {
                None = 0,
                OpenGL = 1,
            };
        public:
            virtual ~RendererAPI() = default;

            virtual void Init() = 0;

            virtual void Clear() = 0;

            virtual void Clear(const glm::vec4& color) = 0;

            // !TODO: change to accept count and depth test flag
            virtual void DrawIndexed(uint32_t count, bool depthTest = true) = 0;

            virtual void DrawArrays(uint32_t mode, uint32_t first, uint32_t count, bool depthTest = true) = 0;

            inline static RenderAPIType GetAPIType() { return s_api_type; }

            inline static RenderAPICapabilities& GetCapabilities() { 
                static RenderAPICapabilities s_capabilities;
                return s_capabilities; 
            }
        private:
            inline static RenderAPIType s_api_type = RendererAPI::RenderAPIType::OpenGL;
    };
}