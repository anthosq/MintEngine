#pragma once

#include <memory>
#include "layer_stack.h"
#include "event/event_system.h"
#include "render/window_system.h"
#include "event/application_event.h"

namespace Mint {
    class Application { 
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer) { m_layer_stack.PushLayer(layer); }
        void PushOverlay(Layer* overlay) { m_layer_stack.PushOverlay(overlay); }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        std::unique_ptr<Window> m_window;
        bool m_running = true;
        LayerStack m_layer_stack;
    };

    // to be defined in client
    std::shared_ptr<Application> CreateApplication();

}