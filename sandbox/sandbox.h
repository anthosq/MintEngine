#include <iostream>
#include "engine.h"

class ExampleLayer : public Mint::Layer {
public:
    ExampleLayer();
    // 生命周期Methods
    void OnUpdate(Mint::TimeStep delta_time) override;
    void OnEvent(Mint::Event& e) override;
    void OnImGuiRender() override;

    // 初始化Utils
    void SetupShaders();
    void SetupBuffers();
    void SetupTextures();
    void SetupFramebuffers() {}

    // 渲染Utils
    // void RenderScene();
    void OnRender(Mint::TimeStep delta_time);
    void RenderCubes();
    void RenderSkybox();
    void RenderTransparent();

private:
    Mint::ShaderLibrary m_shader_library;

    Mint::Ref<Mint::Shader> m_plane_shader;
    Mint::Ref<Mint::VertexArray> m_plane_vao;
    Mint::Ref<Mint::Texture2D> m_plane_texture;

    // Cube
    Mint::Ref<Mint::Shader> m_cube_shader;
    Mint::Ref<Mint::VertexArray> m_cube_vao;
    Mint::Ref<Mint::Texture2D> m_cube_texture;

    // test
    Mint::Ref<Mint::Texture2D> m_transparent_texture;

    // camera
    Mint::EditorCamera m_camera;
    glm::vec3 rectangle_transform;
    glm::vec3 rectangle_color = glm::vec3(0.2f, 0.3f, 0.8f);

    // skybox
    Mint::Ref<Mint::Shader> m_skybox_shader;
    Mint::Ref<Mint::TextureCube> m_skybox_texture, m_skybox_irradiance_texture;

    // framebuffer
    Mint::Ref<Mint::Framebuffer> m_framebuffer;
};


class Sandbox : public Mint::Application {
public:
    Sandbox() {
        PushLayer(new ExampleLayer());
        // PushOverlay(new Mint::ImGuiLayer());
    };
    ~Sandbox() override = default;
};