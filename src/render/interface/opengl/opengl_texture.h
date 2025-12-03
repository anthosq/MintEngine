#pragma once

#include "render/texture.h"
#include "stb_image.h"
#include <string>

#include "core/buffer.h"

namespace Mint{
    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D(const TextureSpecification& spec, const std::filesystem::path& path);
        OpenGLTexture2D(const TextureSpecification& spec);
        // OpenGLTexture2D(unsigned int width, unsigned int height);
        virtual ~OpenGLTexture2D();

        void Bind(uint32_t slot) const override;

        unsigned int GetWidth() const override { return m_specification.Width; }
        unsigned int GetHeight() const override { return m_specification.Height; }
        unsigned int GetRendererID() const override { return m_rendererID; }
        TextureType GetType() const override { return TextureType::Texture2D; }
        TextureFormat GetFormat() const override { return m_specification.Format; }
        glm::vec2 GetSize() const override { return glm::vec2{ (float)m_specification.Width, (float)m_specification.Height }; }

        bool IsLoaded() const override { return m_isLoaded; }

        const std::filesystem::path& GetPath() const override { return m_path; }
        void CreateFromFile(const TextureSpecification& spec, const std::filesystem::path& path) override;

    private:
        std::filesystem::path m_path;
        unsigned int m_rendererID;
        TextureSpecification m_specification;
        Buffer m_image_data;

        // TODO: 后续完善Asset系统, 通过存储的Image判断是否载入
        bool m_isLoaded;
    };

    class OpenGLTextureCube : public TextureCube {
    public:
        OpenGLTextureCube(const TextureSpecification& spec, const std::filesystem::path& path);
        virtual ~OpenGLTextureCube();

        void Bind(uint32_t slot) const override;

        unsigned int GetWidth() const override { return m_specification.Width; }
        unsigned int GetHeight() const override { return m_specification.Height; }
        unsigned int GetRendererID() const override { return m_rendererID; }
        TextureType GetType() const override { return TextureType::TextureCube; }
        TextureFormat GetFormat() const override { return m_specification.Format; }
        glm::vec2 GetSize() const override { return glm::vec2{ (float)m_specification.Width, (float)m_specification.Height }; }
    
        // CreateFromFile?

        const std::filesystem::path& GetPath() const override { return m_path; }
    private:
        std::filesystem::path m_path;
        unsigned int m_rendererID;
        TextureSpecification m_specification;
    };
}