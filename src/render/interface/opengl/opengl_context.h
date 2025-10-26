#include "render/interface/opengl/gl_common.h"
#include "render/graphics_context.h"

namespace Mint {
    class OpenGLContext : public GraphicContext {
        public:
            OpenGLContext(GLFWwindow* window_handle);
            virtual ~OpenGLContext();

            void Init();
            void SwapBuffers();

        private:
            GLFWwindow *m_window_handle;
    
    };
}
