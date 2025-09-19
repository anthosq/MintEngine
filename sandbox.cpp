#include <iostream>
#include "engine.h"

class ExampleLayer : public Mint::Layer {
public:
    ExampleLayer() : Layer("Example") {};

    void OnUpdate() override {
        Mint::LOG_INFO("ExampleLayer::OnUpdate");
    }

    void OnEvent(Mint::Event& e) override {
        Mint::LOG_INFO("ExampleLayer::OnEvent");
    }
};

class Sandbox : public Mint::Application {
public:
    Sandbox() {
        // PushLayer(new ExampleLayer());
    };
    ~Sandbox() override = default;
};

std::shared_ptr<Mint::Application> Mint::CreateApplication() {
    return std::make_shared<Sandbox>();
}