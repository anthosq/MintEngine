#include "global_context.h"
#include "Application.h"


namespace Mint {
    RuntimeGlobalContext g_runtime_global_context;

    void RuntimeGlobalContext::startSystems() {

        m_log_system = std::make_shared<LogSystem>();
        LOG_INFO("Log system initialized.");
    
    }

    void RuntimeGlobalContext::shutdownSystems() {
        if (m_application) {
            m_application.reset();
            LOG_INFO("Application shut down.");
        }
        m_log_system.reset();
        LOG_INFO("Log system shut down.");
    }

}
