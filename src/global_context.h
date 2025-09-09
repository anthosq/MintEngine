#pragma once
#include <memory>
#include "Application.h"

namespace Mint {
    class Application;

    class RuntimeContext {
    public:
        std::shared_ptr<Application> m_application;

    };

    extern RuntimeContext g_runtime_global_context;
}