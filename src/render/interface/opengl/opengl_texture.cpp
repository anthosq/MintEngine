#include "opengl_texture.h"
#include "render/interface/opengl/gl_common.h"
#include "render/interface/opengl/opengl_utils.h"

#include "log_system.h"

namespace Mint {
    // temporary implementation
    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& spec)
        : m_specification(spec), m_isLoaded(false) {
        
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& spec, const std::filesystem::path& path)
        : m_specification(spec), m_path(path)
    {
        CreateFromFile(spec, path);
    }


    void OpenGLTexture2D::CreateFromFile(const TextureSpecification& spec, const std::filesystem::path& path) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

        if (!data) {
            m_isLoaded = false;
            LOG_ERROR(fmt::format("Failed to load texture image from path: {0}", path.string()));
            return;
        }

        m_specification.Width = width;
        m_specification.Height = height;
        m_isLoaded = true;

        GLenum internalFormat = 0, dataFormat = 0;

        if (m_specification.Format != TextureFormat::None) {
            internalFormat = Utils::ToGLTextureFormat(m_specification.Format);
            dataFormat = internalFormat == GL_RGB8 ? GL_RGB : GL_RGBA;
        } else {
            Utils::InferFormatFromChannels(channels, internalFormat, dataFormat);
            m_specification.Format = (dataFormat == GL_RGB) ? TextureFormat::RGB8 : TextureFormat::RGBA8;
        }

        LOG_INFO(fmt::format("Loaded texture {0} ({1}x{2}, {3} channels)", path.string(), width, height, channels));
        LOG_INFO(fmt::format("Inferred format: internalFormat={0}, dataFormat={1}", internalFormat, dataFormat));

        glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
        glTextureStorage2D(m_rendererID, 1, internalFormat, m_specification.Width, m_specification.Height);

        // currently without mipmaps
        glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, Utils::ToGLTextureFilter(m_specification.MinFilter));
        glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, Utils::ToGLTextureFilter(m_specification.MagFilter));

        glTextureSubImage2D(m_rendererID, 0, 0, 0, m_specification.Width, m_specification.Height,
                            dataFormat, GL_UNSIGNED_BYTE, data);

        if (m_specification.GenerateMipMaps) {
            glGenerateTextureMipmap(m_rendererID);
        }

        stbi_image_free(data);
    }



    OpenGLTexture2D::~OpenGLTexture2D() {
        glDeleteTextures(1, &m_rendererID);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const {
        glBindTextureUnit(slot, m_rendererID);
    }

    // OpenGLTextureCube implementation would go here
    // !!!TODO: temporary implementation
    OpenGLTextureCube::OpenGLTextureCube(const TextureSpecification& spec, const std::filesystem::path& path)
        : m_specification(spec), m_path(path)
    {
        // TODO: Implement cube texture loading
    }

    OpenGLTextureCube::~OpenGLTextureCube() {
        glDeleteTextures(1, &m_rendererID);
    }

    void OpenGLTextureCube::Bind(uint32_t slot) const {
        glBindTextureUnit(slot, m_rendererID);
    }

}