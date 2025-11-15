#include "opengl_renderer_api.h"

namespace Mint {
    void OpenGLRendererAPI::Init() {
        // Enable depth testing
        // glEnable(GL_DEPTH_TEST);
        // glDepthFunc(GL_LESS);

        // Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable face culling
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
    }

    void OpenGLRendererAPI::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    void OpenGLRendererAPI::Clear(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
 
    void OpenGLRendererAPI::DrawIndexed(unsigned int count, bool depth_test) {
        if (!depth_test) {
            glDisable(GL_DEPTH_TEST);
        } 

        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

        if (!depth_test) {
            glEnable(GL_DEPTH_TEST);
        }
    }
}