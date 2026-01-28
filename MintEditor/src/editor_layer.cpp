#include "editor_layer.h"
#include <map>


namespace Mint {
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


EditorLayer::EditorLayer() : Layer("Example"), m_camera(60, 1600.0f / 900.0f, 0.1f, 100.0f), // 16:9 纵横比
                               rectangle_transform({0.0f, 0.0f, 0.0f}) {
    // Mint::LOG_INFO("ExampleLayer::OnAttach");

    // Prepare something for render
    SetupShaders();
    SetupBuffers();
    SetupTextures();

}

void EditorLayer::OnUpdate(Mint::TimeStep delta_time) {
    // Mint::LOG_INFO("ExampleLayer::OnUpdate");
    // 后续处理动作逻辑可以采用这种轮询的方式？
    // if (Mint::Input::IsKeyPressed(Mint::Key::A)) {
    //     Mint::LOG_INFO("Key A is pressed (polling)");
    OnRender(delta_time);

}

void EditorLayer::OnEvent(Mint::Event& e) {
    // Mint::LOG_INFO("ExampleLayer::OnEvent");
    // if (e.GetEventType() == Mint::EventType::KeyPressed) {
    //     Mint::KeyPressedEvent& event = (Mint::KeyPressedEvent&)e;
    //     Mint::LOG_INFO(fmt::format("KeyPressedEvent: {0} ({1} repeats)", event.GetKeyCode(), event.GetRepeatCount()));

    // }
    // Mint::EventDispatcher dispatcher(e);
    // dispatcher.Dispatch<Mint::KeyPressedEvent>(BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
    m_camera.OnEvent(e);
}

void EditorLayer::OnImGuiRender() {
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Mint Editor DockSpace", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
                g_runtime_global_context.m_application->Close();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    ImGui::SetCursorPos(ImVec2(10, 10));

    if (viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f) {
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
    }

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    // 关键：如果不阻止 Input，鼠标在 ImGui 窗口操作时也会移动 3D 相机
    // Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

    m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

    uint32_t textureID = m_framebuffer->GetColorAttachmentRendererID();
    ImGui::Image((void *)(uintptr_t)textureID, ImVec2{1280.0f, 720.0f}, ImVec2{0, 1}, ImVec2{1, 0});

    ImGui::End();
    ImGui::PopStyleVar();

    // --- 其他面板 ---
    ImGui::Begin("Camera Stats");
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
    ImGui::Text("Viewport Size: %.1f, %.1f", viewportPanelSize.x, viewportPanelSize.y);
    ImGui::End();
    ImGui::End(); // End DockSpace
}


// Setup Utils
void EditorLayer::SetupShaders() {
    m_plane_shader = Mint::Shader::Create("sandbox/assets/shaders/test_shader.glsl");
    m_shader_library.Load("sandbox/assets/shaders/test_texture.glsl");
    m_shader_library.Load("sandbox/assets/shaders/cube_shader.glsl");
}

void EditorLayer::SetupBuffers() {

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


    // Test plane
    m_plane_vao = Mint::VertexArray::Create();

    Mint::Ref<Mint::VertexBuffer> m_plane_vbo;
    Mint::Ref<Mint::IndexBuffer> m_plane_ibo;
    m_plane_vbo = Mint::VertexBuffer::Create(vertices, sizeof(vertices));

    Mint::BufferLayout layout = {
        {Mint::ShaderDataType::Float3, "a_position"},
        {Mint::ShaderDataType::Float2, "a_texCoord"}};
    m_plane_vbo->SetLayout(layout);
    m_plane_vao->AddVertexBuffer(m_plane_vbo);

    unsigned int indices[6] = {0, 1, 2, 0, 3, 2};
    m_plane_ibo = Mint::IndexBuffer::Create(indices, sizeof(indices));
    m_plane_vao->SetIndexBuffer(m_plane_ibo);

    // ScreenQuad VAO

    // framebuffer
    m_framebuffer = Mint::Framebuffer::Create({.Width = 1280, .Height = 720, .Attachments = {Mint::FramebufferTextureSpecification(Mint::FramebufferTextureFormat::RGBA8),
                                                                                                      Mint::FramebufferTextureSpecification(Mint::FramebufferTextureFormat::DEPTH24STENCIL8)}});

}

void EditorLayer::SetupTextures() {

    m_plane_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/Checkerboard.png");
    m_transparent_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/ChernoLogo.png");
    m_cube_texture = Mint::Texture2D::Create({.MagFilter = Mint::TextureFilter::Nearest}, "sandbox/assets/pics/Container2.png");
    // m_skybox_texture = Mint::TextureCube::Create("sandbox/assets/pics/Arches_E_PineTree_Radiance.tga");
    // m_skybox_irradiance_texture = Mint::TextureCube::Create("sandbox/assets/pics/Arches_E_PineTree_Irradiance.tga");

}
// void SetupFramebuffers()

// Render Utils
// void ExampleLayer::RenderScene() {

// }
void EditorLayer::OnRender(Mint::TimeStep delta_time) {


    m_framebuffer->Bind();
    Mint::g_runtime_global_context.m_render_system->Clear({0.1f, 0.1f, 0.1f, 1});


    // Need to complete scene, adding ECS here

    if (m_ViewportFocused) {
        m_camera.OnUpdate(delta_time);
    }

    Mint::g_runtime_global_context.m_render_system->BeginScene(m_camera);

    RenderCubes();

    RenderSkybox();

    // TODO: Move to OnRender function
    // temporary transform function
    // for each object, compute transform matrix

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), rectangle_transform);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    transform = transform * scale;

    m_plane_texture->Bind();
    auto m_texture_shader = m_shader_library.Get("test_texture");
    m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
    Mint::g_runtime_global_context.m_render_system->Submit(m_texture_shader, m_plane_vao, transform);

    m_transparent_texture->Bind();
    m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
    Mint::g_runtime_global_context.m_render_system->Submit(m_texture_shader, m_plane_vao, transform, false);

    Mint::g_runtime_global_context.m_render_system->EndScene();
    m_framebuffer->Unbind();
    Mint::g_runtime_global_context.m_render_system->Clear({0.1f, 0.1f, 0.1f, 1});
}


void EditorLayer::RenderCubes() {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    transform = transform * scale;

    m_cube_texture->Bind();
    auto m_cube_shader = m_shader_library.Get("cube_shader");
    m_cube_shader->SetInt("u_Texture", 0); // Texture unit 1
    Mint::g_runtime_global_context.m_render_system->SubmitArrays(m_cube_shader, m_cube_vao, GL_TRIANGLES, 36, 0, transform);
}

void EditorLayer::RenderSkybox() {


}

// 透明物体由远到近绘制
void EditorLayer::RenderTransparent() {
    // 透明物体绘制流程

    // std::map<float, glm::vec3> sorted;
    // glm::vec3 camPos = m_camera.GetPosition();
    // for (const auto& pos : m_transparent_positions) {
    //     float distance = glm::length(camPos - pos);
    //     sorted[distance] = pos;
    // }

    // for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    //     glm::mat4 model = glm::translate(glm::mat4(1.0f), it->second);
    //     model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    //     m_transparent_texture->Bind();
    //     auto m_texture_shader = m_shader_library.Get("test_texture");
    //     m_texture_shader->SetInt("u_Texture", 0); // Texture unit 0
    //     Mint::g_runtime_global_context.m_render_system->Submit(m_texture_shader, m_plane_vao, model, false);
    // }
}
}