#pragma once

#include "global_context.h"
#include "Application.h"

// int main(int argc, char** argv) {
//     g_runtime_global_context.m_application = std::make_shared<Mint::Application>();
//     g_runtime_global_context.m_application->Run();
//     return 0;
// }

extern Mint::RuntimeGlobalContext Mint::g_runtime_global_context;
extern std::shared_ptr<Mint::Application> Mint::CreateApplication();

int main() {
    Mint::g_runtime_global_context.startSystems();
    Mint::g_runtime_global_context.m_application = Mint::CreateApplication();
    // Mint::LOG_INFO("Application created.");
    // Mint::LOG_WARN("This is a warning message.");
    // Mint::LOG_ERROR("This is an error message.");
    // Mint::LOG_DEBUG("This is a debug message.");
    // Mint::LOG_FATAL("This is a fatal message.");
    Mint::LOG_INFO("Mint Engine started.");
    Mint::g_runtime_global_context.m_application->Run();
    return 0;
}