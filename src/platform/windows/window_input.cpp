#include "window_input.h"

#include "Application.h"
#include "render/interface/opengl/gl_common.h"

namespace Mint {
    
    Input *Input::s_Instance = new WindowInput();

    // TODO: 重新实现Input系统, 使其不依赖GLFW的keycode定义
    bool WindowInput::IsKeyPressedImpl(KeyCode keycode) {
        auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, static_cast<int>(keycode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool WindowInput::IsMouseButtonPressedImpl(MouseCode button) {
        auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<int>(button));
        return state == GLFW_PRESS;
    }

    std::pair<float, float> WindowInput::GetMousePositionImpl() {
        auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return {(float)xpos, (float)ypos};
    }

    float WindowInput::GetMouseXImpl()
    {
        auto [x, y] = GetMousePositionImpl();
        return x;
    }

    float WindowInput::GetMouseYImpl() {
        auto [x, y] = GetMousePositionImpl();
        return y;
    }
}