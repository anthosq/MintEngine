#pragma once

#include "precompile.h"
#include "layer_stack.h"
#include "event/event_system.h"
#include "render/window_system.h"
#include "event/application_event.h"
#include "imgui/imgui_layer.h"

#include "render/interface/opengl/opengl_shader.h"
#include "render/render_system.h"
#include "render/buffer.h"
#include "render/vertex_array.h"
#include "render/render_system.h"

namespace Mint {
    class Application { 
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        inline Window& GetWindow() { return *m_window; }
        inline static Application& GetInstance() { return *s_instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        std::unique_ptr<Window> m_window;
        ImGuiLayer* m_imgui_layer;
        bool m_running = true;
        LayerStack m_layer_stack;
        static Application* s_instance;

        // 临时
        std::shared_ptr<VertexArray> m_vertex_array;
        std::shared_ptr<Shader> m_shader;
    };

    // to be defined in client
    std::shared_ptr<Application> CreateApplication();

}