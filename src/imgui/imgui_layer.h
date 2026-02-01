#pragma once
#include "layer.h"

// 临时添加
#include <imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

namespace Mint {

    class ImGuiLayer : public Layer {
        public:
            ImGuiLayer();
            ~ImGuiLayer() = default;

            virtual void OnAttach() override;
            virtual void OnDetach() override;

            virtual void OnEvent(Event& event) override;

            // ImGUI层不采用OnUpdate形式更新, 因为可能其他层也需要绘制ImGui
            // 因此采用Begin/End形式进行管理
            void Begin();
            void End();

            virtual void OnImGuiRender() override;

            void BlockEvents(bool block) { m_blockEvents = block; }
            uint32_t GetActiveWindowID() const; 
        private:
            bool m_blockEvents = true;

    };
}