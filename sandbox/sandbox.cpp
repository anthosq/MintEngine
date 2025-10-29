#include <iostream>
#include "engine.h"

class ExampleLayer : public Mint::Layer {
public:
    ExampleLayer() : Layer("Example"), m_camera(-1.6f, 1.6f, -0.9f, 0.9f), // 16:9 纵横比
                     camera_position(0.0f, 0.0f, 0.0f), camera_rotation(0.0f), rectangle_transform({0.0f, 0.0f, 0.0f}) { 
        // Mint::LOG_INFO("ExampleLayer::OnAttach");

        // -------------临时------------------
        // Prepare something for render
        // Vertex Array
        // Index Buffer

        m_vertex_array = Mint::VertexArray::Create();

        // adding texture coordinates
        float vertices[4 * 5] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

        // vertex buffer
        Mint::Ref<Mint::VertexBuffer> m_vertex_buffer;
        Mint::Ref<Mint::IndexBuffer> m_index_buffer;
        m_vertex_buffer = Mint::VertexBuffer::Create(vertices, sizeof(vertices));

        Mint::BufferLayout layout = {
            {Mint::ShaderDataType::Float3, "a_position"},
            {Mint::ShaderDataType::Float2, "a_texCoord"}
        };
        m_vertex_buffer->SetLayout(layout);

        m_vertex_array->AddVertexBuffer(m_vertex_buffer);

        // index buffer
        unsigned int indices[6] = {0, 1, 2, 0, 3, 2};
        m_index_buffer = Mint::IndexBuffer::Create(indices, 6);
        m_vertex_array->SetIndexBuffer(m_index_buffer);

        // shader wrap seems completed
        m_shader = Mint::Shader::Create("sandbox/assets/shaders/test_shader.glsl");

        m_shader_library.Load("sandbox/assets/shaders/test_texture.glsl");

        Mint::TextureSpecification spec;
        m_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/Checkerboard.png");
        m_transparent_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/ChernoLogo.png");
    }

    void OnUpdate(Mint::TimeStep delta_time) override {
        // Mint::LOG_INFO("ExampleLayer::OnUpdate");
        // 后续处理动作逻辑可以采用这种轮询的方式？
        // if (Mint::Input::IsKeyPressed(Mint::Key::A)) {
        //     Mint::LOG_INFO("Key A is pressed (polling)");

        // Mint::LOG_INFO(fmt::format("Delta Time: {0}", delta_time.GetSeconds()));
        // camera movement
        if (Mint::Input::IsKeyPressed(Mint::Key::W)) {
            camera_position.y += camera_move_speed * delta_time;
        }
        else if(Mint::Input::IsKeyPressed(Mint::Key::S)) {
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

        // temporary transform function
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), rectangle_transform);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
        transform = transform * scale;

        // for (int i = 0; i < 10; i++) {
        //     float offset = i * 1.1f;
        //     glm::mat4 square_transform = glm::translate(transform, glm::vec3(offset, 0.0f, 0.0f));
        //     m_shader->Bind();
        //     m_shader->SetFloat4("u_Color", glm::vec4(rectangle_color, 1.0f));
        //     Mint::RenderSystem::Submit(m_shader, m_vertex_array, square_transform);
        // }

        m_texture->Bind();
        auto m_texture_shader = m_shader_library.Get("test_texture");
        m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
        Mint::RenderSystem::Submit(m_texture_shader, m_vertex_array, transform);

        m_transparent_texture->Bind();
        m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
        Mint::RenderSystem::Submit(m_texture_shader, m_vertex_array, transform);

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
        ImGui::Begin("Settings");
        ImGui::ColorEdit3("Rectangle Color", glm::value_ptr(rectangle_color));
        
        ImGui::End();
    }

    private:
    Mint::Ref<Mint::Shader> m_shader;
    Mint::Ref<Mint::VertexArray> m_vertex_array;
    Mint::Ref<Mint::Texture2D> m_texture;
    Mint::ShaderLibrary m_shader_library;

    // test
    Mint::Ref<Mint::Texture2D> m_transparent_texture;

    // camera
    Mint::Camera m_camera;
    glm::vec3 camera_position;
    float camera_rotation;
    float camera_move_speed = 0.1f;
    float camera_rotation_speed = 2.0f;

    glm::vec3 rectangle_transform;
    glm::vec3 rectangle_color = glm::vec3(0.2f, 0.3f, 0.8f);
    
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