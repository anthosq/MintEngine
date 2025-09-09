#include <iostream>
#include "engine.h"

class Sandbox : public Mint::Application {
public:
    Sandbox() = default;
    ~Sandbox() override = default;
};

std::shared_ptr<Mint::Application> Mint::CreateApplication() {
    return std::make_shared<Sandbox>();
}