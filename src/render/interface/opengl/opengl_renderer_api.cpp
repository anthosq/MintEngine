#include "opengl_renderer_api.h"

namespace Mint {
    void OpenGLRendererAPI::Init() {

        // Get OpenGL capabilities
        RenderAPICapabilities& capabilities = RendererAPI::GetCapabilities();
        capabilities.Vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        capabilities.Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        capabilities.Version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        glGetIntegerv(GL_MAX_SAMPLES, &capabilities.MaxSamples);
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &capabilities.MaxAnisotropy);

        // Enable depth testing
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);

        // Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable face culling
		glFrontFace(GL_CCW);
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
    }

    void OpenGLRendererAPI::Clear() {
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    void OpenGLRendererAPI::Clear(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
 
    void OpenGLRendererAPI::DrawIndexed(unsigned int count, bool depth_test) {
        if (!depth_test) {
            glDisable(GL_DEPTH_TEST);
        } else {
            glEnable(GL_DEPTH_TEST);
        }

        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

        if (!depth_test) {
            glEnable(GL_DEPTH_TEST);
        }
    }

    void OpenGLRendererAPI::DrawArrays(uint32_t mode, uint32_t first, uint32_t count, bool depth_test) {
        if (!depth_test) {
            glDisable(GL_DEPTH_TEST);
        } else {
            glEnable(GL_DEPTH_TEST);
        }

        glDrawArrays(mode, first, count);

        if (!depth_test) {
            glEnable(GL_DEPTH_TEST);
        }
    }
}