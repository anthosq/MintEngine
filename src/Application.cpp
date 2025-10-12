#include "Application.h"
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

        // -------------临时------------------
        // Prepare something for render
        // Vertex Array
        // Index Buffer


        glGenVertexArrays(1, &m_vertex_array);
        glBindVertexArray(m_vertex_array);

        float vertices[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.0f, 0.2f, 0.8f, 0.0f, 1.0f,
             0.0f,  0.5f, 0.0f, 0.1f, 0.1f, 0.8f, 1.0f
        };

        m_vertex_buffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

        {      
            BufferLayout layout = {
                { ShaderDataType::Float3, "a_position" },
                { ShaderDataType::Float4, "a_color" }
            };
            m_vertex_buffer->SetLayout(layout);
        }
        
        uint32_t index = 0;
        // buffer layout
        for (auto const& element: m_vertex_buffer->GetLayout()) {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, 
                element.GetComponentCount(),
                GetOpenGLDataType(element.type),
                element.normalized ? GL_TRUE : GL_FALSE,
                m_vertex_buffer->GetLayout().GetStride(),
                (const void*)element.offset);
            index++;
        }


        // index buffer
        unsigned int indices[3] = { 0, 1, 2 };
        m_index_buffer.reset(IndexBuffer::Create(indices, 3));


        std::string vertex_src = R"(
            #version 330 core

            layout(location = 0) in vec3 a_position;
            layout(location = 1) in vec4 a_color;

            out vec3 v_position;
            out vec4 v_color;
            void main()
            {
                v_position = a_position;
                v_color = a_color;
                gl_Position = vec4(a_position, 1.0);
            }
        )";
        std::string fragment_src = R"(
            #version 330 core

            layout(location = 0) out vec4 color;

            in vec3 v_position;
            in vec4 v_color;

            void main()
            {
                color = vec4(v_position * 0.5 + 0.5, 1.0);
            }
        )";
        m_shader = std::make_unique<Shader>(vertex_src, fragment_src);
        // ---------------------------------
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
            glClearColor(0.1f, 0.1f, 0.1f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_shader->Bind();
            glBindVertexArray(m_vertex_array);
            glDrawElements(GL_TRIANGLES, m_index_buffer->GetCount(), GL_UNSIGNED_INT, nullptr);



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