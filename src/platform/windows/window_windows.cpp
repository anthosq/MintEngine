#include "window_windows.h"

#include "event/key_event.h"
#include "event/mouse_event.h"
#include "log_system.h"


namespace Mint {
    static uint8_t s_GLFWWindowCount = 0;

    Window* Window::Create(const WindowCreateInfo& create_info)
    {
        return new WindowsWindow(create_info);
    }

    WindowsWindow::WindowsWindow(const WindowCreateInfo& create_info)
    {
        Init(create_info);
    }

    WindowsWindow::~WindowsWindow()
    {
        Shutdown();
    }

    void WindowsWindow::OnUpdate()
    {
        glfwPollEvents();
    }

    std::array<unsigned int, 2> WindowsWindow::GetWindowSize() const
    {
        return { m_data.width, m_data.height };
    }

    void WindowsWindow::SetEventCallback(const EventCallbackFn& callback)
    {
        m_data.event_callback = callback;
    }

    void WindowsWindow::SetVsync(bool enabled)
    {
        m_data.v_sync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    bool WindowsWindow::IsVsync() const
    {
        return m_data.v_sync;
    }

    bool WindowsWindow::getFocusMode() const
    {
        return m_data.m_is_focus;
    }

    void WindowsWindow::setFocusMode(bool is_focus)
    {
        m_data.m_is_focus = is_focus;
        glfwSetInputMode(m_window, GLFW_CURSOR, m_data.m_is_focus ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    void WindowsWindow::Init(const WindowCreateInfo& create_info)
    {
        m_data.title = create_info.title;
        m_data.width = create_info.width;
        m_data.height = create_info.height;

        
        if (!glfwInit())
        {
            LOG_FATAL("Could not initialize GLFW!");
            return;
        }

        LOG_INFO(fmt::format("Creating window {0} ({1}, {2})", m_data.title, m_data.width, m_data.height));

        m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
        if (!m_window)
        {
            glfwTerminate();
            // TODO: Handle window creation failure
        }

        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, &m_data);
        SetVsync(true);
    }

    void WindowsWindow::Shutdown()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

}
