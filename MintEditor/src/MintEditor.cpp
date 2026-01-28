#pragma once
#include "EntryPoint.h"
#include "editor_layer.h"
#include "engine.h"

class MintEditor : public Mint::Application {
public:
    MintEditor(const Mint::ApplicationProps& props) : Mint::Application(props) {
        PushLayer(new Mint::EditorLayer());
    }

    ~MintEditor() {}
};

std::shared_ptr<Mint::Application> Mint::CreateApplication()
{
    return std::make_shared<MintEditor>(Mint::ApplicationProps{"Mint Editor", 1600, 900});
}
