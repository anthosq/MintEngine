#pragma once
#include "render/interface/opengl/gl_common.h"
#include "render/texture.h"

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
    }

}