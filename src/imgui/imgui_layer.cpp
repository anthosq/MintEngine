#include "imgui_layer.h"
#include "Application.h"

// 临时添加
#include <imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <glfw/glfw3.h>
#include "log_system.h"
// -----------------    


namespace Mint {
    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer") {
    }

    ImGuiLayer::~ImGuiLayer() {
    }

    void ImGuiLayer::OnAttach() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void) io;
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        GLFWwindow *window = glfwGetCurrentContext();
        if (!window) {
            LOG_ERROR("No current OpenGL context for ImGui!");
            return;
        }
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        io.DisplaySize = ImVec2((float)1920, (float)1080);

        // ImGui更新, 新版本ImGui后端只需要io.addKeyEvent()报告事件, 不再需要io.KeyMap[]
        ImGui_ImplOpenGL3_Init("#version 330");
        LOG_INFO("ImGuiLayer attached.");

    }

    void ImGuiLayer::OnDetach() {
    }

    void ImGuiLayer::OnUpdate() {
        // temporary delta time
        ImGuiIO& io = ImGui::GetIO(); 
        Application& app = Application::GetInstance();
        io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

        float time = (float)glfwGetTime();
        io.DeltaTime = m_last_time > 0.0 ? (time - m_last_time) : (1.0f / 60.0f);
        m_last_time = time;

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnEvent(Event& event) {
    }

    void ImGuiLayer::Begin() {
    }

    void ImGuiLayer::End() {
    }

    
}