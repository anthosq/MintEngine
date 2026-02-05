#pragma once
#include "render/interface/opengl/gl_common.h"
#include "render/texture.h"
#include "render/shader.h"
#include "render/framebuffer.h"

namespace Mint {
    namespace Utils {

        // utils for framebuffer
        static GLenum TOGLFramebufferTexFormat(FramebufferTextureFormat format) {
            switch (format) {
                case Mint::FramebufferTextureFormat::RGBA8:             return GL_RGBA8;
                case Mint::FramebufferTextureFormat::RGBA16F:           return GL_RGBA16F;
                case Mint::FramebufferTextureFormat::RED_INTEGER:       return GL_RED_INTEGER;
                case Mint::FramebufferTextureFormat::DEPTH24STENCIL8:   return GL_DEPTH24_STENCIL8;
            }
            return 0;
        }


        static GLenum TextureTarget(bool multisample) {
            return multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        static void CreateTextures(bool multisample, uint32_t* outID, uint32_t count) {
            glCreateTextures(TextureTarget(multisample), count, outID);
        }

        static void BindTexture(bool multisample, uint32_t id) {
            glBindTexture(TextureTarget(multisample), id);
        }

        static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat,
                                       GLenum format, uint32_t width, uint32_t height, int index) {
            bool multisample = samples > 1;
            if (multisample) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat,
                                        width, height, GL_FALSE);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
                             0, format, GL_UNSIGNED_BYTE, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
                                   TextureTarget(multisample), id, 0);
        }

        static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType,
                                      uint32_t width, uint32_t height) {
            bool multisample = samples > 1;
            if (multisample) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format,
                                        width, height, GL_FALSE);
            } else {
                glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType,
                                   TextureTarget(multisample), id, 0);
        }

        static bool IsDepthFormat(FramebufferTextureFormat format) {
            switch (format) {
                case Mint::FramebufferTextureFormat::DEPTH24STENCIL8:
                    return true;
            }
            return false;
        }

        // utils for texture
        static GLenum ToGLTextureFormat(TextureFormat format){
            switch (format) {
                case Mint::TextureFormat::RGB8:     return GL_RGB8;
                case Mint::TextureFormat::RGBA8:    return GL_RGBA8;
                case Mint::TextureFormat::SRGB8:    return GL_SRGB8;
                case Mint::TextureFormat::SRGB_ALPHA8: return GL_SRGB8_ALPHA8;
            }
            return 0;
        }

        static GLenum ToGLTextureWrap(TextureWrap wrap) {
            switch (wrap) {
                case Mint::TextureWrap::Repeat:         return GL_REPEAT;
                case Mint::TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
                case Mint::TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
            }
            return 0;
        }

        static GLenum ToGLTextureFilter(TextureFilter filter) {
            switch (filter) {
                case Mint::TextureFilter::Linear:   return GL_LINEAR;
                case Mint::TextureFilter::Nearest:  return GL_NEAREST;
            }
            return 0;
        }

        static int CalculateMipMapCount(int width, int height)
        {
            int levels = 1;
            while ((width | height) >> levels) {
                levels++;
            }
            return levels;
        }

        // not sure
        static void InferFormatFromChannels(int channels, GLenum& internalFormat, GLenum& dataFormat) {
            switch (channels) {
                case 3: internalFormat = GL_RGB8; dataFormat = GL_RGB; break;
                case 4: internalFormat = GL_RGBA8; dataFormat = GL_RGBA; break;
                default: internalFormat = 0; dataFormat = 0; break;
            }
            return ;
        }


        static ShaderUniformType GLTypeToShaderUniformType(GLenum type) {
            switch (type) {
                case GL_BOOL: return ShaderUniformType::Bool;
                case GL_INT: return ShaderUniformType::Int;
                case GL_FLOAT: return ShaderUniformType::Float;
                case GL_FLOAT_VEC2: return ShaderUniformType::Vec2;
                case GL_FLOAT_VEC3: return ShaderUniformType::Vec3;
                case GL_FLOAT_VEC4: return ShaderUniformType::Vec4;
                case GL_FLOAT_MAT3: return ShaderUniformType::Mat3;
                case GL_FLOAT_MAT4: return ShaderUniformType::Mat4;
                case GL_SAMPLER_2D: return ShaderUniformType::Int; // Sampler 通常作为 Int 处理 (slot)
            }
            return ShaderUniformType::None;
        }

        static uint32_t GetShaderUniformSize(ShaderUniformType type) {
            switch (type) {
                case ShaderUniformType::Bool:    return sizeof(bool);
                case ShaderUniformType::Int:     return sizeof(int32_t);
                case ShaderUniformType::UInt:    return sizeof(uint32_t);
                case ShaderUniformType::Float:   return sizeof(float);
                case ShaderUniformType::Vec2:    return sizeof(glm::vec2);
                case ShaderUniformType::Vec3:    return sizeof(glm::vec3);
                case ShaderUniformType::Vec4:    return sizeof(glm::vec4);
                case ShaderUniformType::Mat3:    return sizeof(glm::mat3);
                case ShaderUniformType::Mat4:    return sizeof(glm::mat4);
                case ShaderUniformType::IVec2:   return sizeof(glm::ivec2);
                case ShaderUniformType::IVec3:   return sizeof(glm::ivec3);
                case ShaderUniformType::IVec4:   return sizeof(glm::ivec4);
                default:                         return 0;
            }
        }
    }

}