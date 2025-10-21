#include <iostream>
#include "engine.h"

class ExampleLayer : public Mint::Layer {
public:
    ExampleLayer() : Layer("Example"), m_camera(-1.6f, 1.6f, -0.9f, 0.9f), // 16:9 纵横比
                     camera_position(0.0f, 0.0f, 0.0f), camera_rotation(0.0f) { 
        // Mint::LOG_INFO("ExampleLayer::OnAttach");

        // -------------临时------------------
        // Prepare something for render
        // Vertex Array
        // Index Buffer

        m_vertex_array.reset(Mint::VertexArray::Create());

        float vertices[4 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, 0.0f, 0.2f, 0.8f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.0f, 0.1f, 0.1f, 0.8f, 1.0f,
            0.5f, -0.5f, 0.0f, 0.8f, 0.8f, 0.1f, 1.0f};

        // vertex buffer
        std::shared_ptr<Mint::VertexBuffer> m_vertex_buffer;
        std::shared_ptr<Mint::IndexBuffer> m_index_buffer;
        m_vertex_buffer.reset(Mint::VertexBuffer::Create(vertices, sizeof(vertices)));

        Mint::BufferLayout layout = {
            {Mint::ShaderDataType::Float3, "a_position"},
            {Mint::ShaderDataType::Float4, "a_color"}};
        m_vertex_buffer->SetLayout(layout);

        m_vertex_array->AddVertexBuffer(m_vertex_buffer);

        // index buffer
        unsigned int indices[6] = {0, 1, 2, 0, 3, 2};
        m_index_buffer.reset(Mint::IndexBuffer::Create(indices, 6));

        m_vertex_array->SetIndexBuffer(m_index_buffer);
        // drawing rectangle
        std::string vertex_src = R"(
            #version 330 core

            layout(location = 0) in vec3 a_position;
            layout(location = 1) in vec4 a_color;

            uniform mat4 u_ViewProjection;

            out vec3 v_position;
            out vec4 v_color;
            void main()
            {
                v_position = a_position;
                v_color = a_color;
                gl_Position = u_ViewProjection * vec4(a_position, 1.0);
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
        m_shader.reset(Mint::Shader::Create(vertex_src, fragment_src));
        // ---------------------------------
    }

    void OnUpdate(Mint::TimeStep delta_time) override {
        // Mint::LOG_INFO("ExampleLayer::OnUpdate");
        // 后续处理动作逻辑可以采用这种轮询的方式？
        // if (Mint::Input::IsKeyPressed(Mint::Key::A)) {
        //     Mint::LOG_INFO("Key A is pressed (polling)");

        Mint::LOG_INFO(fmt::format("Delta Time: {0}", delta_time.GetSeconds()));
        // camera movement
        if (Mint::Input::IsKeyPressed(Mint::Key::W)) {
            camera_position.y += camera_move_speed * delta_time;
        }
        else if (Mint::Input::IsKeyPressed(Mint::Key::S)) {
            camera_position.y -= camera_move_speed * delta_time;
        }
        if (Mint::Input::IsKeyPressed(Mint::Key::A)) {
            camera_position.x -= camera_move_speed * delta_time;
        }
        else if (Mint::Input::IsKeyPressed(Mint::Key::D)) {
            camera_position.x += camera_move_speed * delta_time;
        }

        if (Mint::Input::IsKeyPressed(Mint::Key::Q)) {
            camera_rotation += camera_rotation_speed * delta_time;
        }
        else if (Mint::Input::IsKeyPressed(Mint::Key::E)) {
            camera_rotation -= camera_rotation_speed * delta_time;
        }

        Mint::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        Mint::RenderCommand::Clear();
        Mint::RenderCommand::ClearDepth();

        // m_camera.SetPosition({0.0f, 0.0f, 0.0f});
        m_camera.SetPosition(camera_position);
        m_camera.SetRotation_deg(camera_rotation);


        Mint::RenderSystem::BeginScene(m_camera);
        Mint::RenderSystem::Submit(m_shader, m_vertex_array);
        Mint::RenderSystem::EndScene();
    }

    void OnEvent(Mint::Event& e) override {
        // Mint::LOG_INFO("ExampleLayer::OnEvent");
        // if (e.GetEventType() == Mint::EventType::KeyPressed) {
        //     Mint::KeyPressedEvent& event = (Mint::KeyPressedEvent&)e;
        //     Mint::LOG_INFO(fmt::format("KeyPressedEvent: {0} ({1} repeats)", event.GetKeyCode(), event.GetRepeatCount()));

        // }
        // Mint::EventDispatcher dispatcher(e);
        // dispatcher.Dispatch<Mint::KeyPressedEvent>(BIND_EVENT_FN(ExampleLayer::OnKeyPressed));

    }

    void OnImGuiRender() override {
    }

    private:
    std::shared_ptr<Mint::Shader> m_shader;
    std::shared_ptr<Mint::VertexArray> m_vertex_array;
    // camera
    Mint::Camera m_camera;
    glm::vec3 camera_position;
    float camera_rotation;
    float camera_move_speed = 0.1f;
    float camera_rotation_speed = 2.0f;
    
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