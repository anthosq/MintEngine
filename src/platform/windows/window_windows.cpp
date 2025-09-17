#include "window_windows.h"

#include "event/key_event.h"
#include "event/mouse_event.h"
#include "event/application_event.h"
#include "log_system.h"


namespace Mint {
    static uint8_t s_GLFWWindowCount = 0;

    static void GLFWErrorCallback(int error, const char* description)
    {
        LOG_ERROR(fmt::format("GLFW Error ({0}): {1}", error, description));
    }

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
            glfwSetErrorCallback(nullptr);
            return;
        }

        LOG_INFO(fmt::format("Creating window {0} ({1}, {2})", m_data.title, m_data.width, m_data.height));

        m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, &m_data);
        SetVsync(true);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.width = width;
            data.height = height;

            WindowResizeEvent event(width, height);
            data.event_callback(event);
        });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent event;
            data.event_callback(event);
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.event_callback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.event_callback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.event_callback(event);
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.event_callback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.event_callback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            MouseScrolledEvent event(xOffset, yOffset);
            data.event_callback(event);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            MouseMovedEvent event(xPos, yPos);
            data.event_callback(event);
        });

    }

    void WindowsWindow::Shutdown()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

}
