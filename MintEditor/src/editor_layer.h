#pragma once
#include "MintEngine/src/layer.h"

namespace Mint {
    class EditorLayer : public Layer {
    public:
        EditorLayer() = default;
        ~EditorLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
    };
}
