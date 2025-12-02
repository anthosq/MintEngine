#include "opengl_texture.h"
#include "render/interface/opengl/gl_common.h"
#include "render/interface/opengl/opengl_utils.h"
#include "render/render_system.h"

#include "log_system.h"

#include <array>

namespace Mint {
    // temporary implementation
    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& spec)
        : m_specification(spec), m_isLoaded(false) {
            RenderSystem::Submit([this]() {
                // generate empty texture
                glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
                glTextureStorage2D(m_rendererID, 1, Utils::ToGLTextureFormat(m_specification.Format), m_specification.Width, m_specification.Height);
                glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, Utils::ToGLTextureFilter(m_specification.MinFilter));
                glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, Utils::ToGLTextureFilter(m_specification.MagFilter));
                glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, Utils::ToGLTextureWrap(m_specification.WrapS));
                glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, Utils::ToGLTextureWrap(m_specification.WrapT));
            });
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

        // LOG_INFO(fmt::format("Loaded texture {0} ({1}x{2}, {3} channels)", path.string(), width, height, channels));
        // LOG_INFO(fmt::format("Inferred format: internalFormat={0}, dataFormat={1}", internalFormat, dataFormat));

        RenderSystem::Submit([this, internalFormat, dataFormat, data]()
                             {
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
        });

        stbi_image_free(data);
    }



    OpenGLTexture2D::~OpenGLTexture2D() {
        RenderSystem::Submit([this]() {
            glDeleteTextures(1, &m_rendererID);
        });
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const {
        RenderSystem::Submit([this, slot]() {
            glBindTextureUnit(slot, m_rendererID);
        });
    }

    // OpenGLTextureCube implementation would go here
    // !!!TODO: temporary implementation
    OpenGLTextureCube::OpenGLTextureCube(const TextureSpecification& spec, const std::filesystem::path& path)
        : m_specification(spec), m_path(path) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(0);
        // TODO: using own image data class, like buffer<uint8_t>
        stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

        if (!data) {
            LOG_ERROR(fmt::format("Failed to load cube texture image from path: {0}", path.string()));
            return;
        }

        m_specification.Width = width;
        m_specification.Height = height;
        m_specification.Format = TextureFormat::RGB8;

        unsigned int faceWidth = width / 4;
        unsigned int faceHeight = height / 3;


        if (faceWidth != faceHeight) {
            LOG_ERROR(fmt::format("Cube texture faces are not square in image: {0}", path.string()));
            stbi_image_free(data);
            return;
        }

        std::array<unsigned char*, 6> faces;
        for (unsigned int i = 0; i < 6; i++) {
            faces[i] = new unsigned char[faceWidth * faceHeight * 3];
        }

        int faceIndex = 0;

        // NOTE: confused, 之后弄清楚后重构
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 4; j++) {
                if ((i == 1 && j == 1) || // +Y
                    (i == 0 && j == 1) || // -Y
                    (i == 1 && j == 0) || // -X
                    (i == 1 && j == 2) || // +X
                    (i == 2 && j == 1) || // +Z
                    (i == 1 && j == 3))   // -Z
                {
                    for (unsigned int y = 0; y < faceHeight; y++) {
                        memcpy(faces[faceIndex] + y * faceWidth * 3,
                               data + ((i * faceHeight + y) * width + (j * faceWidth)) * 3,
                               faceWidth * 3);
                    }
                    faceIndex++;
                }
            }
        }

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);
        glTextureStorage2D(m_rendererID, 1, GL_RGB8, faceWidth, faceHeight);
        for (unsigned int i = 0; i < 6; i++) {
            glTextureSubImage3D(m_rendererID, 0, 0, 0, i, faceWidth, faceHeight, 1,
                                GL_RGB, GL_UNSIGNED_BYTE, faces[i]);
            delete[] faces[i];
        }
        glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, Utils::ToGLTextureFilter(m_specification.MinFilter));
        glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, Utils::ToGLTextureFilter(m_specification.MagFilter));
        stbi_image_free(data);
    }

    OpenGLTextureCube::~OpenGLTextureCube() {
        glDeleteTextures(1, &m_rendererID);
    }

    void OpenGLTextureCube::Bind(uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTextureUnit(slot, m_rendererID);
    }

}