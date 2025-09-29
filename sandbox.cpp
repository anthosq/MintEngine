#include <iostream>
#include "engine.h"

class ExampleLayer : public Mint::Layer {
public:
    ExampleLayer() : Layer("Example") {};

    void OnUpdate() override {
        // Mint::LOG_INFO("ExampleLayer::OnUpdate");
        // 后续处理动作逻辑可以采用这种轮询的方式？
        if (Mint::Input::IsKeyPressed(Mint::Key::A)) {
            Mint::LOG_INFO("Key A is pressed (polling)");
        }
    }

    void OnEvent(Mint::Event& e) override {
        // Mint::LOG_INFO("ExampleLayer::OnEvent");
        if (e.GetEventType() == Mint::EventType::KeyPressed) {
            Mint::KeyPressedEvent& event = (Mint::KeyPressedEvent&)e;
            Mint::LOG_INFO(fmt::format("KeyPressedEvent: {0} ({1} repeats)", event.GetKeyCode(), event.GetRepeatCount()));

        }
    }
};

class Sandbox : public Mint::Application {
public:
    Sandbox() {
        PushLayer(new ExampleLayer());
        // PushOverlay(new Mint::ImGuiLayer());
    };
    ~Sandbox() override = default;
};

std::shared_ptr<Mint::Application> Mint::CreateApplication() {
    return std::make_shared<Sandbox>();
}