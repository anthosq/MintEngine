#include "sandbox.h"

// temporary data for test
static float vertices[4 * 5] = {
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

    // ...existing code...

static float cube_vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
};


ExampleLayer::ExampleLayer() : Layer("Example"), m_camera(60, 1600.0f / 900.0f, 0.1f, 100.0f), // 16:9 纵横比
                               rectangle_transform({0.0f, 0.0f, 0.0f}) {
    // Mint::LOG_INFO("ExampleLayer::OnAttach");

    // Prepare something for render
    SetupShaders();
    SetupBuffers();
    SetupTextures();

}

void ExampleLayer::OnUpdate(Mint::TimeStep delta_time) {
    // Mint::LOG_INFO("ExampleLayer::OnUpdate");
    // 后续处理动作逻辑可以采用这种轮询的方式？
    // if (Mint::Input::IsKeyPressed(Mint::Key::A)) {
    //     Mint::LOG_INFO("Key A is pressed (polling)");
    OnRender(delta_time);

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
void ExampleLayer::SetupShaders() {
    m_shader = Mint::Shader::Create("sandbox/assets/shaders/test_shader.glsl");
    m_shader_library.Load("sandbox/assets/shaders/test_texture.glsl");
    m_shader_library.Load("sandbox/assets/shaders/cube_shader.glsl");
}

void ExampleLayer::SetupBuffers() {

    // Cube
    m_cube_vao = Mint::VertexArray::Create();
    Mint::Ref<Mint::VertexBuffer> cube_vbo;
    // Mint::Ref<Mint::IndexBuffer> cube_ibo;
    cube_vbo = Mint::VertexBuffer::Create(cube_vertices, sizeof(cube_vertices));
    Mint::BufferLayout cube_layout = {
        {Mint::ShaderDataType::Float3, "a_Position"},
        {Mint::ShaderDataType::Float3, "a_Normal"},
        {Mint::ShaderDataType::Float2, "a_TexCoord"}};
    cube_vbo->SetLayout(cube_layout);
    m_cube_vao->AddVertexBuffer(cube_vbo);
    // cube_ibo = Mint::IndexBuffer::Create(cube_indices, sizeof(cube_indices));
    // m_cube_vao->SetIndexBuffer(cube_ibo);


    // Test vao&vbo&ibo
    m_vertex_array = Mint::VertexArray::Create();

    Mint::Ref<Mint::VertexBuffer> m_vertex_buffer;
    Mint::Ref<Mint::IndexBuffer> m_index_buffer;
    m_vertex_buffer = Mint::VertexBuffer::Create(vertices, sizeof(vertices));

    Mint::BufferLayout layout = {
        {Mint::ShaderDataType::Float3, "a_position"},
        {Mint::ShaderDataType::Float2, "a_texCoord"}};
    m_vertex_buffer->SetLayout(layout);
    m_vertex_array->AddVertexBuffer(m_vertex_buffer);

    unsigned int indices[6] = {0, 1, 2, 0, 3, 2};
    m_index_buffer = Mint::IndexBuffer::Create(indices, sizeof(indices));
    m_vertex_array->SetIndexBuffer(m_index_buffer);

    // ScreenQuad VAO

}

void ExampleLayer::SetupTextures() {

    m_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/Checkerboard.png");
    m_transparent_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/ChernoLogo.png");
    m_cube_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/Container2.png");
    // m_skybox_texture = Mint::TextureCube::Create("sandbox/assets/pics/Arches_E_PineTree_Radiance.tga");
    // m_skybox_irradiance_texture = Mint::TextureCube::Create("sandbox/assets/pics/Arches_E_PineTree_Irradiance.tga");

}
// void SetupFramebuffers()

// Render Utils
// void ExampleLayer::RenderScene() {

// }
void ExampleLayer::OnRender(Mint::TimeStep delta_time) {
    // Not sure whether this should be here
    Mint::g_runtime_global_context.m_render_system->Clear({0.1f, 0.1f, 0.1f, 1});

    // Need to complete scene, adding ECS here
    m_camera.OnUpdate(delta_time);
    Mint::g_runtime_global_context.m_render_system->BeginScene(m_camera);

    RenderCubes();

    RenderSkybox();

    // TODO: Move to OnRender function
    // temporary transform function
    // for each object, compute transform matrix

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), rectangle_transform);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    transform = transform * scale;

    m_texture->Bind();
    auto m_texture_shader = m_shader_library.Get("test_texture");
    m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
    Mint::g_runtime_global_context.m_render_system->Submit(m_texture_shader, m_vertex_array, transform);

    m_transparent_texture->Bind();
    m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
    Mint::g_runtime_global_context.m_render_system->Submit(m_texture_shader, m_vertex_array, transform);

    Mint::g_runtime_global_context.m_render_system->EndScene();
}


void ExampleLayer::RenderCubes() {

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    transform = transform * scale;

    m_cube_texture->Bind();
    auto m_cube_shader = m_shader_library.Get("cube_shader");
    m_cube_shader->SetInt("u_Texture", 0); // Texture unit 1
    Mint::g_runtime_global_context.m_render_system->SubmitArrays(m_cube_shader, m_cube_vao, GL_TRIANGLES, 36, 0, transform);
}
void ExampleLayer::RenderSkybox() {


}

std::shared_ptr<Mint::Application> Mint::CreateApplication() {
    return std::make_shared<Sandbox>();
}