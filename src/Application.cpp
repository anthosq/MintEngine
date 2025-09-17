#include "Application.h"
#include <memory>
#include "log_system.h"


namespace Mint {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

    Application::Application() {
        m_window = std::unique_ptr<Window>(Window::Create(WindowCreateInfo()));
        // 成员函数指针与std::function不兼容, std::function需要一个可调用对象
        // 所以成员指针需要使用std::bind创一个新的可调用对象
        // 或者使用lambda [this](Event& e) { this->OnEvent(e); }
        m_window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
    }

    Application::~Application() {
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
    }

    void Application::Run() {
        while(m_running) {
            m_window->OnUpdate();
        }

        g_runtime_global_context.shutdownSystems();
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        m_running = false;
        LOG_INFO("Window close event received, stopping application.");
        return true;
    }
}