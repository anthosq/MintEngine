#include "Application.h"
#include <memory>
#include "event/application_event.h"
#include "log_system.h"


namespace Mint {

    Application::Application() {
    }

    Application::~Application() {
    }

    void Application::Run() {
        m_window = std::unique_ptr<Window>(Window::Create(WindowCreateInfo()));
        while(m_running) {
            m_window->OnUpdate();
        }
    }

}