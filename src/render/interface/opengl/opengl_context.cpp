#include "opengl_context.h"
#include "../../gl_common.h"
#include "../../../log_system.h"


namespace Mint {
    OpenGLContext::OpenGLContext(GLFWwindow* window_handle)
        : m_window_handle(window_handle) {}

    OpenGLContext::~OpenGLContext() {}

    void OpenGLContext::Init() {
        glfwMakeContextCurrent(m_window_handle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        MT_ASSERT(status, "Failed to initialize Glad!");

        // 注意后续调整LOG_INFO宏定义, 使其支持fmt格式化字符串
        LOG_INFO("OpenGL Info:");
        LOG_INFO(fmt::format("  Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
        LOG_INFO(fmt::format("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
        LOG_INFO(fmt::format("  Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION))));    
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_window_handle);
    }

}
