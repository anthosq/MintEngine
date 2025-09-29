#pragma once
#include "layer.h"


namespace Mint {

    class ImGuiLayer : public Layer {
        public:
            ImGuiLayer();
            ~ImGuiLayer() override;

            void OnAttach() override;
            void OnDetach() override;

            void OnUpdate() override;
            void OnEvent(Event& event) override;

            void Begin();
            void End();
        private:
            // temporary
            float m_last_time = 0.0f;
    };
}