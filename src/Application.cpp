#include "Application.h"
#include <memory>
#include "log_system.h"
#include "input.h"
#include <glm/glm.hpp>

// 临时头文件, 后续需要将clear部分转移
#include "./render/gl_common.h"

namespace Mint {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

    Application *Application::s_instance = nullptr;

    Application::Application() {
        s_instance = this;
        m_window = std::unique_ptr<Window>(Window::Create(WindowCreateInfo()));
        // 成员函数指针与std::function不兼容, std::function需要一个可调用对象
        // 所以成员指针需要使用std::bind创一个新的可调用对象
        // 或者使用lambda [this](Event& e) { this->OnEvent(e); }
        m_window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
        // 所有权问题, 小心处理
        m_imgui_layer = new ImGuiLayer();
        PushOverlay(m_imgui_layer);
    }

    Application::~Application() {
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        // LOG_INFO(fmt::format("Event: {0}", e.ToString()));
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

        for (auto it = m_layer_stack.end(); it != m_layer_stack.begin(); ) {
            (*--it)->OnEvent(e);
            if (e.IsHandled()) {
                break;
            }
        }
    }

    void Application::Run() {
        while (m_running) {
            // Actually happening in frame buffer
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (Layer* layer : m_layer_stack) {
                layer->OnUpdate();
            }

            // On the render
            m_imgui_layer->Begin();
            for (Layer* layer : m_layer_stack) {
                layer->OnImGuiRender();
            }
            m_imgui_layer->End();
            // auto [x,y] = Input::GetMousePosition();
            // LOG_INFO(fmt::format("Mouse Position: ({0}, {1})", x, y));
            m_window->OnUpdate();
        }
        g_runtime_global_context.shutdownSystems();
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        m_running = false;
        LOG_INFO("Window close event received, stopping application.");
        return true;
    }

    void Application::PushLayer(Layer* layer) {
        m_layer_stack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* overlay) {
        m_layer_stack.PushOverlay(overlay);
        overlay->OnAttach();
    }
}