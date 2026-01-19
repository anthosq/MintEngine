#include "texture.h"
#include "render/renderer_api.h"
#include "render/interface/opengl/opengl_texture.h"

// TODO: Assert宏需要完善
// CreateRef, temporary use std::make_shared

namespace Mint {
    Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec, const std::filesystem::path& path) {
        switch (RendererAPI::GetAPIType()) {
            case RendererAPI::RenderAPIType::None:            return nullptr;
            // case RendererAPI::API::OpenGL:          return Ref<OpenGLTexture2D>::CreateFromFile(spec, path);
            case RendererAPI::RenderAPIType::OpenGL:          return Ref<OpenGLTexture2D>::Create(spec, path);
        }

        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec) {
        switch (RendererAPI::GetAPIType()) {
            case RendererAPI::RenderAPIType::None:            return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:          return Ref<OpenGLTexture2D>::Create(spec);
        }

        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<TextureCube> TextureCube::Create(const std::filesystem::path& path) {
        switch (RendererAPI::GetAPIType()) {
            case RendererAPI::RenderAPIType::None:            return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:          // TODO: 实现OpenGLTextureCube类
                                                    return Ref<OpenGLTextureCube>::Create(path);
        }

        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}