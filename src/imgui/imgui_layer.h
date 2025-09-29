#pragma once
#include "layer.h"


namespace Mint {

    class ImGuiLayer : public Layer {
        public:
            ImGuiLayer();
            ~ImGuiLayer() = default;

            virtual void OnAttach() override;
            virtual void OnDetach() override;

            virtual void OnEvent(Event& event) override;

            void Begin();
            void End();


        private:
            // temporary
            float m_last_time = 0.0f;
    };
}