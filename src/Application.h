#pragma once

#include <memory>
#include "event/event_system.h"

namespace Mint {

    class Application { 
    public:
        Application();
        virtual ~Application();

        void Run();
    };

    // to be defined in client
    std::shared_ptr<Application> CreateApplication();

}