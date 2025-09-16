#pragma once

#include <memory>
#include "event/event_system.h"
#include "render/window_system.h"

namespace Mint {

    class Application { 
    public:
        Application();
        virtual ~Application();

        void Run();

    private:
        std::unique_ptr<Window> m_window;
        bool m_running = true;
    };

    // to be defined in client
    std::shared_ptr<Application> CreateApplication();

}