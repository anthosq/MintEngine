#pragma once

#include <memory>
#include "event/event_system.h"
#include "render/window_system.h"
#include "event/application_event.h"

namespace Mint {

    class Application { 
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        std::unique_ptr<Window> m_window;
        bool m_running = true;
    };

    // to be defined in client
    std::shared_ptr<Application> CreateApplication();

}