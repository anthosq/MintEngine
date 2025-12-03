#pragma once

#include "precompile.h"
#include "layer_stack.h"
#include "event/event_system.h"
#include "render/window_system.h"
#include "event/application_event.h"
#include "imgui/imgui_layer.h"

#include "render/shader.h"
#include "render/render_system.h"
#include "render/buffer.h"
#include "render/vertex_array.h"
#include "render/render_system.h"

#include "render/camera.h"
#include "core/time_step.h"

#include "Core.h"

#include <chrono>
namespace Mint {
    class Application { 
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);
        void OnImGuiRender();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        inline Window& GetWindow() { return *m_window; }
        inline static Application& GetInstance() { return *s_instance; }

        float CalculateDeltaTime();

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
        std::unique_ptr<Window> m_window;
        ImGuiLayer* m_imgui_layer;
        bool m_running = true;
        LayerStack m_layer_stack;
        static Application* s_instance;
        // float m_last_tick_time = 0.0f;
        std::chrono::steady_clock::time_point m_last_tick_time = std::chrono::steady_clock::now();

        // temporary
        bool m_minimized = false;

    };

    // to be defined in client
    std::shared_ptr<Application> CreateApplication();

}