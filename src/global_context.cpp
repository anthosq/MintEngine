#include "global_context.h"
#include "Application.h"


namespace Mint {
    RuntimeGlobalContext g_runtime_global_context;

    void RuntimeGlobalContext::startSystems() {

        m_log_system = std::make_shared<LogSystem>();
        LOG_INFO("Log system initialized.");

        m_render_system = std::make_shared<RenderSystem>();
    }

    void RuntimeGlobalContext::shutdownSystems() {
        if (m_application) {
            m_application.reset();
            LOG_INFO("Application shut down.");
        }
        LOG_INFO("Log system shut down.");
        RefUtils::DumpLiveRefs();
        m_log_system.reset();
        // m_render_system.shutdown();
        m_render_system.reset();
    }

}
