#include "Application.h"
#include "Core.h"
#include "log_system.h"
#include "input.h"
#include <glm/glm.hpp>

// 临时头文件, 后续需要将clear部分转移

namespace Mint {

    Application *Application::s_instance = nullptr;

    Application::Application() {
        s_instance = this;
        // 之后会重命名Application为MintEngine, 并且把构造函数内的部分内容移到Init函数中
        // window, renderer之类的子系统转移由RuntimeGlobalContext进行管理, 进行初始化
        m_window = std::unique_ptr<Window>(Window::Create(WindowCreateInfo()));
        // 成员函数指针与std::function不兼容, std::function需要一个可调用对象
        // 所以成员指针需要使用std::bind创一个新的可调用对象
        // 或者使用lambda [this](Event& e) { this->OnEvent(e); }
        m_window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        // 所有权问题, 小心处理
        m_imgui_layer = new ImGuiLayer();
        PushOverlay(m_imgui_layer);

        g_runtime_global_context.m_render_system->Init();
        g_runtime_global_context.m_render_system->WaitAndRender();

    }

    Application::~Application() {
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        // LOG_INFO(fmt::format("Event: {0}", e.ToString()));
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        for (auto it = m_layer_stack.end(); it != m_layer_stack.begin(); ) {
            (*--it)->OnEvent(e);
            if (e.IsHandled()) {
                break;
            }
        }
    }

    float Application::CalculateDeltaTime()
    {
        float delta_time;
        {
            using namespace std::chrono;
            steady_clock::time_point tick_time = steady_clock::now();
            duration<float> time_span = duration_cast<duration<float>>(tick_time - m_last_tick_time);
            delta_time = time_span.count();
            m_last_tick_time = tick_time;
        }
        return delta_time; 
    }

    void Application::Run() {
        TimeStep delta_time = CalculateDeltaTime();

        while (m_running) {
            if (!m_minimized) {
                for (Layer *layer : m_layer_stack) {
                    layer->OnUpdate(delta_time);
                }

                // TODO: render imgui on render thread
                // Application may need to create a RenderImGui() function
                // To handle imgui layer
            }

            // On the render
            m_imgui_layer->Begin();
            for (Layer* layer : m_layer_stack) {
                layer->OnImGuiRender();
            }
            m_imgui_layer->End();
            // auto [x,y] = Input::GetMousePosition();
            // LOG_INFO(fmt::format("Mouse Position: ({0}, {1})", x, y));
            g_runtime_global_context.m_render_system->WaitAndRender();
            m_window->OnUpdate();
        }
        g_runtime_global_context.shutdownSystems();
    }


    bool Application::OnWindowClose(WindowCloseEvent &e)
    {
        m_running = false;
        LOG_INFO("Window close event received, stopping application.");
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        g_runtime_global_context.m_render_system->OnWindowResize(e.GetWidth(), e.GetHeight());
        LOG_INFO(fmt::format("Window resized to: ({0}, {1})", e.GetWidth(), e.GetHeight()));
        if (e.GetWidth() == 0 || e.GetHeight() == 0) {
            m_minimized = true;
            return false;
        }
        m_minimized = false;
        return false;
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