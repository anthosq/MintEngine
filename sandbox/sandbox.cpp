#include "sandbox.h"

ExampleLayer::ExampleLayer() : Layer("Example"), m_camera(60, 1600.0f / 900.0f, 0.1f, 100.0f), // 16:9 纵横比
                               rectangle_transform({0.0f, 0.0f, 0.0f}) {
    // Mint::LOG_INFO("ExampleLayer::OnAttach");

    // -------------临时------------------
    // Prepare something for render
    // Vertex Array
    // Index Buffer

    m_vertex_array = Mint::VertexArray::Create();

    // adding texture coordinates
    float vertices[4 * 5] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

    // vertex buffer
    Mint::Ref<Mint::VertexBuffer> m_vertex_buffer;
    Mint::Ref<Mint::IndexBuffer> m_index_buffer;
    m_vertex_buffer = Mint::VertexBuffer::Create(vertices, sizeof(vertices));

    Mint::BufferLayout layout = {
        {Mint::ShaderDataType::Float3, "a_position"},
        {Mint::ShaderDataType::Float2, "a_texCoord"}};
    m_vertex_buffer->SetLayout(layout);

    m_vertex_array->AddVertexBuffer(m_vertex_buffer);

    // index buffer
    unsigned int indices[6] = {0, 1, 2, 0, 3, 2};
    m_index_buffer = Mint::IndexBuffer::Create(indices, 6 * sizeof(unsigned int));
    m_vertex_array->SetIndexBuffer(m_index_buffer);

    // shader wrap seems completed
    m_shader = Mint::Shader::Create("sandbox/assets/shaders/test_shader.glsl");

    m_shader_library.Load("sandbox/assets/shaders/test_texture.glsl");

    Mint::TextureSpecification spec;
    m_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/Checkerboard.png");
    m_transparent_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/ChernoLogo.png");

    // skybox
    m_skybox_texture = Mint::TextureCube::Create("sandbox/assets/pics/Arches_E_PineTree_Radiance.tga");
    m_skybox_irradiance_texture = Mint::TextureCube::Create("sandbox/assets/pics/Arches_E_PineTree_Irradiance.tga");

    // // test for skybox
    // Mint::TextureSpecification cubeSpec;
    // cubeSpec.MinFilter = Mint::TextureFilter::Linear;
    // cubeSpec.MagFilter = Mint::TextureFilter::Linear;
    // cubeSpec.WrapS = Mint::TextureWrap::ClampToEdge;
    // cubeSpec.WrapT = Mint::TextureWrap::ClampToEdge;

    // m_skybox_texture = Mint::TextureCube::Create(cubeSpec, "sandbox/assets/pics/Arches_E_PineTree_Radiance.tga");
    // m_shader_library.Load("sandbox/assets/shaders/test_skybox.glsl");
    // Mint::Ref<Mint::VertexBuffer> skyboxVB = Mint::VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices));
    // Mint::BufferLayout skyboxLayout = {
    //     {Mint::ShaderDataType::Float3, "a_Position"}
    // };
    // skyboxVB->SetLayout(skyboxLayout);
    // m_skybox_vertex_array = Mint::VertexArray::Create();
    // m_skybox_vertex_array->AddVertexBuffer(skyboxVB);
}

void ExampleLayer::OnUpdate(Mint::TimeStep delta_time) {
    // Mint::LOG_INFO("ExampleLayer::OnUpdate");
    // 后续处理动作逻辑可以采用这种轮询的方式？
    // if (Mint::Input::IsKeyPressed(Mint::Key::A)) {
    //     Mint::LOG_INFO("Key A is pressed (polling)");

    // Mint::LOG_INFO(fmt::format("Delta Time: {0}", delta_time.GetSeconds()));
    // camera movement

    Mint::g_runtime_global_context.m_render_system->Clear({0.1f, 0.1f, 0.1f, 1});

    m_camera.OnUpdate(delta_time);
    // m_camera.SetPosition({0.0f, 0.0f, 0.0f});

    // m_camera.SetPosition(camera_position);
    // m_camera.SetRotation_deg(camera_rotation);

    Mint::g_runtime_global_context.m_render_system->BeginScene(m_camera);

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
    Mint::g_runtime_global_context.m_render_system->Submit(m_texture_shader, m_vertex_array, transform);

    m_transparent_texture->Bind();
    m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
    Mint::g_runtime_global_context.m_render_system->Submit(m_texture_shader, m_vertex_array, transform);

    // // render skybox
    // auto m_skybox_shader = m_shader_library.Get("test_skybox");;
    // m_skybox_shader->Bind();
    // m_skybox_texture->Bind(0);
    // m_skybox_shader->SetInt("u_Skybox", 0); // Texture unit 0
    // m_skybox_shader->SetMat4("u_View", glm::mat4(glm::mat3(m_camera.GetViewMatrix()))); // remove translation from the view matrix
    // m_skybox_shader->SetMat4("u_Projection", m_camera.GetProjectionMatrix());

    // Mint::RenderSystem::Submit(m_skybox_shader, m_skybox_vertex_array, glm::mat4(1.0f));

    Mint::g_runtime_global_context.m_render_system->EndScene();
}

void ExampleLayer::OnEvent(Mint::Event& e) {
    // Mint::LOG_INFO("ExampleLayer::OnEvent");
    // if (e.GetEventType() == Mint::EventType::KeyPressed) {
    //     Mint::KeyPressedEvent& event = (Mint::KeyPressedEvent&)e;
    //     Mint::LOG_INFO(fmt::format("KeyPressedEvent: {0} ({1} repeats)", event.GetKeyCode(), event.GetRepeatCount()));

    // }
    // Mint::EventDispatcher dispatcher(e);
    // dispatcher.Dispatch<Mint::KeyPressedEvent>(BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
    m_camera.OnEvent(e);
}

void ExampleLayer::OnImGuiRender() {
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Rectangle Color", glm::value_ptr(rectangle_color));

    ImGui::Separator();
    ImGui::Text("Camera Debug");
    auto pos = m_camera.GetPosition();
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    ImGui::Text("Distance: %.2f", m_camera.GetDistance());
    ImGui::Text("Yaw: %.2f, Pitch: %.2f",
                glm::degrees(m_camera.GetYaw()),
                glm::degrees(m_camera.GetPitch()));
    ImGui::Text("Camera Mode: %s",
                m_camera.GetCameraMode() == Mint::CameraMode::FLYCAM ? "FLYCAM" : "ARCBALL");
    ImGui::Text("Focus Point: (%.2f, %.2f, %.2f)",
                m_camera.GetFocalPoint().x,
                m_camera.GetFocalPoint().y,
                m_camera.GetFocalPoint().z);
    ImGui::End();
}

// Setup Utils
void ExampleLayer::SetupShaders() {}
void ExampleLayer::SetupBuffers() {
    // Cube VAO

    // ScreenQuad VAO

}

void ExampleLayer::SetupTextures() {

}
// void SetupFramebuffers()

// Render Utils
void ExampleLayer::RenderScene() {

}

void ExampleLayer::RenderCubes() {

}
void ExampleLayer::RenderSkybox() {


}

std::shared_ptr<Mint::Application> Mint::CreateApplication() {
    return std::make_shared<Sandbox>();
}