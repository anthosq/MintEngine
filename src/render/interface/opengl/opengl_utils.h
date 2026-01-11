#pragma once
#include "render/interface/opengl/gl_common.h"
#include "render/texture.h"
#include "render/shader.h"

namespace Mint {
    namespace Utils {
        static GLenum ToGLTextureFormat(TextureFormat format){
            switch (format)
            {
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