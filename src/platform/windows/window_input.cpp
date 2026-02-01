#include "Application.h"
#include "render/interface/opengl/gl_common.h"
#include "keycodes.h"
#include "input.h"


namespace Mint {
    
    // TODO: 重新实现Input系统, 使其不依赖GLFW的keycode定义
    bool Input::IsKeyPressed(KeyCode keycode) {
        auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, static_cast<int>(keycode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(MouseCode button) {
        auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<int>(button));
        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePosition() {
        auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return {(float)xpos, (float)ypos};
    }

    float Input::GetMouseX()
    {
        auto [x, y] = GetMousePosition();
        return x;
    }

    float Input::GetMouseY() {
        auto [x, y] = GetMousePosition();
        return y;
    }
}