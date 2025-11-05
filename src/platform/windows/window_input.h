#pragma once
#include "input.h"

namespace Mint {
    class WindowInput : public Input {
    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) override;
        virtual bool IsMouseButtonPressedImpl(MouseCode button) override;
        virtual std::pair<float, float> GetMousePositionImpl() override;
        virtual float GetMouseXImpl() override;
        virtual float GetMouseYImpl() override;
    };
}