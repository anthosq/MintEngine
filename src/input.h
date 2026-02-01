#pragma once

#include <utility>
#include "keycodes.h"
#include "mousecodes.h"

namespace Mint {
    class Input {
        public:
            static bool IsKeyPressed(KeyCode keycode);
            static bool IsMouseButtonPressed(MouseCode button);
            static std::pair<float, float> GetMousePosition();
            static float GetMouseX();
            static float GetMouseY();

    };
}