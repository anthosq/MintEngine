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
            auto self = this;
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
        m_image_data.Data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

        if (!m_image_data.Data) {
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

        RenderSystem::Submit([this, internalFormat, dataFormat]()
                             {
            glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
            glTextureStorage2D(m_rendererID, 1, internalFormat, m_specification.Width, m_specification.Height);

            // currently without mipmaps
            glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, Utils::ToGLTextureFilter(m_specification.MinFilter));
            glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, Utils::ToGLTextureFilter(m_specification.MagFilter));

            glTextureSubImage2D(m_rendererID, 0, 0, 0, m_specification.Width, m_specification.Height,
                                dataFormat, GL_UNSIGNED_BYTE, m_image_data.Data);

            if (m_specification.GenerateMipMaps) {
                glGenerateTextureMipmap(m_rendererID);
            }
            stbi_image_free(m_image_data.Data);
        });
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

        for (size_t i = 0; i < 4; i++) {
            for (size_t y = 0; y < faceHeight; y++) {
                size_t yOffset = y + faceHeight;
                for (size_t x = 0; x < faceWidth; x++) {
                    size_t xOffset = x + i * faceWidth;
                    faces[faceIndex][(x + y * faceWidth) * 3 + 0] = data[(xOffset + yOffset * width) * 3 + 0];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 1] = data[(xOffset + yOffset * width) * 3 + 1];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 2] = data[(xOffset + yOffset * width) * 3 + 2];
                }
            }
            faceIndex++;
        }

        for (size_t i = 0; i < 3; i++) {
            if (i == 1) continue;
            for (size_t y = 0; y < faceHeight; y++) {
                size_t yOffset = y + i * faceHeight;
                for (size_t x = 0; x < faceWidth; x++) {
                    size_t xOffset = x + faceWidth;
                    faces[faceIndex][(x + y * faceWidth) * 3 + 0] = data[(xOffset + yOffset * width) * 3 + 0];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 1] = data[(xOffset + yOffset * width) * 3 + 1];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 2] = data[(xOffset + yOffset * width) * 3 + 2];
                }
            }
            faceIndex++;
        }

        RenderSystem::Submit([=]() {
        glGenTextures(1, &m_rendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(m_rendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

        GLenum GLformat = Utils::ToGLTextureFormat(m_specification.Format);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GLformat, faceWidth, faceHeight, 0, GLformat, GL_UNSIGNED_BYTE, faces[2]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GLformat, faceWidth, faceHeight, 0, GLformat, GL_UNSIGNED_BYTE, faces[0]);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GLformat, faceWidth, faceHeight, 0, GLformat, GL_UNSIGNED_BYTE, faces[4]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GLformat, faceWidth, faceHeight, 0, GLformat, GL_UNSIGNED_BYTE, faces[5]);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GLformat, faceWidth, faceHeight, 0, GLformat, GL_UNSIGNED_BYTE, faces[1]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GLformat, faceWidth, faceHeight, 0, GLformat, GL_UNSIGNED_BYTE, faces[3]);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glBindTexture(GL_TEXTURE_2D, 0);

        for (size_t i = 0; i < faces.size(); i++) {
            delete[] faces[i];
        }

        stbi_image_free(data);
        });
    }

    OpenGLTextureCube::~OpenGLTextureCube() {
        auto self = this;
        RenderSystem::Submit([self]() {
            glDeleteTextures(1, &self->m_rendererID);
        });
    }

    void OpenGLTextureCube::Bind(uint32_t slot) const {
        RenderSystem::Submit([this, slot]() {
            glBindTextureUnit(slot, m_rendererID);
        });
    }

}