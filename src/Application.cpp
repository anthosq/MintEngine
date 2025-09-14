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
        WindowResizeEvent e(1280, 720);
        LOG_INFO(e.ToString());
        while(true) {
        }
    }

}