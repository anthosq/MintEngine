#include "material.h"
#include "renderer_api.h"
#include "render/render_system.h"
#include "render/interface/opengl/opengl_material.h"


namespace Mint {
    // Material will be managed by MaterialAsset like ShaderLibrary
    Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::None:
                // MINT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLMaterial>::Create(shader, name);
        }
        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name) {
        switch (RenderSystem::GetAPI()) {
            case RendererAPI::RenderAPIType::None:
                // MINT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:
                return Ref<OpenGLMaterial>::Create(other, name);
        }
        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}