#include "opengl_context.h"
#include "../../gl_common.h"
#include "../../../log_system.h"


namespace Mint {
    OpenGLContext::OpenGLContext(GLFWwindow* window_handle)
        : m_window_handle(window_handle) {}

    OpenGLContext::~OpenGLContext() {}

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_window_handle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        MT_ASSERT(status, "Failed to initialize Glad!");
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_window_handle);
    }

}
