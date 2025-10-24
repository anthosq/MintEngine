#pragma once

#include "../../texture.h"
#include <string>

namespace Mint{
    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D(const std::string& path);
        // OpenGLTexture2D(unsigned int width, unsigned int height);
        virtual ~OpenGLTexture2D();

        void Bind(uint32_t slot) const override;

        unsigned int GetWidth() const override { return m_Width; }
        unsigned int GetHeight() const override { return m_Height; }
        unsigned int GetRendererID() const override { return m_RendererID; }

    private:
        std::string m_Path;
        unsigned int m_Width, m_Height;
        unsigned int m_RendererID;
    };

    class OpenGLTextureCube : public TextureCube {
    public:

    
    };
}