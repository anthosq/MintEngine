#pragma once

#include "src/global_context.h"

// int main(int argc, char** argv) {
//     g_runtime_global_context.m_application = std::make_shared<Mint::Application>();
//     g_runtime_global_context.m_application->Run();
//     return 0;
// }

extern Mint::RuntimeGlobalContext Mint::g_runtime_global_context;
extern std::shared_ptr<Mint::Application> Mint::CreateApplication();

int main() {
    Mint::g_runtime_global_context.m_application = Mint::CreateApplication();
    Mint::g_runtime_global_context.startSystems();
    Mint::LOG_INFO("Application created.");
    std::cout << "Mint Engine initialized!" << std::endl;
    Mint::g_runtime_global_context.m_application->Run();
    return 0;
}