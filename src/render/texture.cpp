#include "texture.h"
#include "renderer_api.h"
#include "interface/opengl/opengl_texture.h"

// TODO: Assert宏需要完善

namespace Mint {
    Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec, const std::filesystem::path& path) {
        switch (RendererAPI::GetAPI()) {
            case RendererAPI::API::None:            return nullptr;
            case RendererAPI::API::OpenGL:          return Ref<OpenGLTexture2D>::Create(spec, path);
        }

        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec) {
        switch (RendererAPI::GetAPI()) {
            case RendererAPI::API::None:            return nullptr;
            case RendererAPI::API::OpenGL:          return Ref<OpenGLTexture2D>::Create(spec);
        }

        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<TextureCube> TextureCube::Create(const TextureSpecification& spec, const std::filesystem::path& path) {
        switch (RendererAPI::GetAPI()) {
            case RendererAPI::API::None:            return nullptr;
            case RendererAPI::API::OpenGL:          // TODO: 实现OpenGLTextureCube类
                                                    return Ref<OpenGLTextureCube>::Create(spec, path);
        }

        // MINT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}