#pragma once
#include <memory>
#include "Application.h"
#include "log_system.h"

namespace Mint {
    class Application;
    class LogSystem;

    class RuntimeGlobalContext {
    /*
    // TODO: METHODS to be added later
    public:
        void startSystems(const std::string& config_file_path);
        void shutdownSystems();
    */
    public:
    void startSystems();
    void shutdownSystems();

    public:
        // possible global systems: logger, input, file, config, asset, world, physics
        // render, particle, debugdraw, render_debug_config ...

        std::shared_ptr<Application> m_application;
        std::shared_ptr<LogSystem> m_log_system;

    };

    extern RuntimeGlobalContext g_runtime_global_context;
}