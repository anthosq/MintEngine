#pragma once

#include "src/global_context.h"

// int main(int argc, char** argv) {
//     g_runtime_global_context.m_application = std::make_shared<Mint::Application>();
//     g_runtime_global_context.m_application->Run();
//     return 0;
// }

Mint::RuntimeContext g_runtime_global_context;
extern std::shared_ptr<Mint::Application> Mint::CreateApplication();

int main() {
    g_runtime_global_context.m_application = Mint::CreateApplication();
    std::cout << "Mint Engine initialized!" << std::endl;
    g_runtime_global_context.m_application->Run();
    return 0;
}