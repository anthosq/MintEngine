#include "editor_camera.h"
#include "Core.h"
#include "mousecodes.h"
#include "input.h"
#include <glm/gtc/quaternion.hpp>
#include <algorithm>

namespace Mint {
    // View与Projection矩阵分开储存, Camera基类只负责Projection矩阵的计算与储存
    // 需要考虑orthognal camera 的实现
    // EditorCamera负责View矩阵的计算与储存以及交互逻辑
    EditorCamera::EditorCamera(float degFov, float aspectRatio, float nearClip, float farClip)
        : Camera(glm::perspective(glm::radians(degFov), aspectRatio, farClip, nearClip),
                 glm::perspective(glm::radians(degFov), aspectRatio, nearClip, farClip))
    {
        // for test

        // test
        m_vertical_fov = degFov;
        m_aspect_ratio = aspectRatio;
        m_nearClip = nearClip;
        m_farClip = farClip;

        m_active = true;
        Init();
    }

    void EditorCamera::Init() {
        // 预设定一个观察角度
        constexpr glm::vec3 position = { -5.0f, 5.0f, 5.0f };
        m_focal_point = glm::vec3(0.0f);
        m_distance = glm::distance(position, m_focal_point);

        // 预设定欧拉角
        m_yaw = glm::radians(135.0f);
        m_pitch = glm::radians(45.0f);

        m_position = CalculatePosition();
        const glm::quat orientation = GetOrientation();
        m_direction = GetForwardDirection();
        m_viewmatrix = glm::translate(glm::mat4(1.0f), m_position) * glm::mat4_cast(orientation);
        // 相机的view就是相机的world矩阵的逆矩阵
        m_viewmatrix = glm::inverse(m_viewmatrix);
    }

    // Disable cursor
    static void DisableCursor() {
        // Implementation to disable cursor
    }

    static void EnableCursor() {
        // Implementation to enable cursor
    }


    void EditorCamera::OnUpdate(TimeStep delta_time) {
        // 这个函数的设计需要调整, 解耦为UpdateFlyCam和UpdateArcball, 目前写法太混乱
        const glm::vec2& mouse_pos = {Input::GetMouseX(), Input::GetMouseY()};
        // sensitivity
        const glm::vec2 delta = (mouse_pos - m_last_mouse_position) * 0.002f;

        if (!m_active) {
            // TODO: 后续设计editor与ImGUI utility时, 启用ImGUI操作UI后再作调整
            m_last_mouse_position = mouse_pos;
            return;
        }

        if (Input::IsMouseButtonPressed(MouseCode::ButtonRight) && !Input::IsKeyPressed(KeyCode::LeftAlt)) {
            // TODO: 移动到UpdateFlyCam函数?
            m_camera_mode = CameraMode::FLYCAM;
            DisableCursor();
            const float speed = GetCameraMoveSpeed();
            const float yaw_sign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

            if (Input::IsKeyPressed(Mint::KeyCode::W)) {
                m_position_delta += delta_time.GetSeconds() * speed * m_direction;
            }
            if (Input::IsKeyPressed(Mint::KeyCode::S)) {
                m_position_delta -= delta_time.GetSeconds() * speed * m_direction;
            }
            if (Input::IsKeyPressed(Mint::KeyCode::A)) {
                m_position_delta -= delta_time.GetSeconds() * speed * m_right_direction;
            }
            if (Input::IsKeyPressed(Mint::KeyCode::D)) {
                m_position_delta += delta_time.GetSeconds() * speed * m_right_direction;
            }
            if (Input::IsKeyPressed(Mint::KeyCode::Q)) {
                m_position_delta -= delta_time.GetSeconds() * speed * glm::vec3(0.0f, yaw_sign, 0.0f);
            }
            if (Input::IsKeyPressed(Mint::KeyCode::E)) {
                m_position_delta += delta_time.GetSeconds() * speed * glm::vec3(0.0f, yaw_sign, 0.0f);
            }

            constexpr float max_rate = 0.1f;
            m_yaw_delta += glm::clamp(yaw_sign * delta.x * RotationSpeed(), -max_rate, max_rate);
            m_pitch_delta += glm::clamp(delta.y * RotationSpeed(), -max_rate, max_rate);
            m_right_direction = glm::cross(m_direction, glm::vec3(0.0f, yaw_sign, 0.0f));

            const float distance = glm::distance(m_focal_point, m_position);
            m_focal_point = m_position + GetForwardDirection() * distance;
            m_distance = distance;
        } else if (Input::IsKeyPressed(KeyCode::LeftAlt)) {
            m_camera_mode = CameraMode::ARCBALL;
            if (Input::IsMouseButtonPressed(MouseCode::ButtonMiddle)) {
                DisableCursor();
                MousePan(delta);
            } else if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft)) {
                DisableCursor();
                MouseRotate(delta);
            } else if (Input::IsMouseButtonPressed(MouseCode::ButtonRight)) {
                DisableCursor();
                MouseZoom((delta.x + delta.y) * 0.1f);
            } else {
                EnableCursor();
            }

        } else {
            EnableCursor();
        }

        m_last_mouse_position = mouse_pos;
        m_position += m_position_delta;
        m_yaw += m_yaw_delta;
        m_pitch += m_pitch_delta;

        if (m_camera_mode == CameraMode::ARCBALL) {
            m_position = CalculatePosition();
            }

        UpdateCameraView();
    }

    void EditorCamera::UpdateCameraView() {
        const float yaw_sign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
        const float cos_angle = glm::dot(GetForwardDirection(), GetUpDirection());
        if (cos_angle * yaw_sign > 0.99f) {
            m_pitch_delta = 0.0f;
        }

        const glm::vec3 target = m_position + GetForwardDirection();
        m_direction = glm::normalize(target - m_position);
        m_distance = glm::distance(m_position, m_focal_point);
        m_viewmatrix = glm::lookAt(m_position, target, glm::vec3{0.0f, yaw_sign, 0.0f});

        // 阻尼设置
        m_yaw_delta *= 0.6f;
        m_pitch_delta *= 0.6f;
        m_position_delta *= 0.8f;
    }



    void EditorCamera::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCamera::OnMouseScroll));
    }

    // !!!! TODO: Focus需要调整, 当Position在FocalPoint后面时会出现问题
    // 这部分问题在实现了color picking后应该会解决, 因为ARC mode会将focus_point设置为选定的物体
    void EditorCamera::Focus(const glm::vec3& focus_point) {
        m_focal_point = focus_point;
        m_camera_mode = CameraMode::FLYCAM;
        if (m_distance > m_min_focus_distance) {
            m_distance -= m_min_focus_distance;
            m_position = m_focal_point - GetForwardDirection() * m_distance;
        }
        m_position = m_focal_point - GetForwardDirection() * m_distance;
        UpdateCameraView();
    }

    // Speed
    float EditorCamera::ZoomSpeed() const {
        float distance = m_distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 50.0f); // max speed
        speed = std::max(speed, 2.0f); // min speed
        return speed;
    }

    float EditorCamera::RotationSpeed() const {
        return 0.8f;
    }

    // magic numbers?
    std::pair<float, float> EditorCamera::PanSpeed() const {
        float x = std::min(m_viewport_width / 1000.0f, 2.4f); // max 2.4f
        float x_factor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = std::min(m_viewport_height / 1000.0f, 2.4f); // max 2.4f
        float y_factor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        return { x_factor, y_factor };
    }

    float EditorCamera::GetCameraMoveSpeed() const {
        float speed = m_normal_camera_move_speed;
        // maxspeed = 2.0f
        if (Input::IsKeyPressed(KeyCode::LeftControl))
            // speed /= 2 - glm::log(m_normal_camera_move_speed);
            speed *= 0.5f;
        if(Input::IsKeyPressed(KeyCode::LeftShift))
            // speed *= 2 + glm::log(m_normal_camera_move_speed);
            speed *= 2.0f;
        return glm::clamp(speed, min_camera_move_speed, max_camera_move_speed);
    }

    bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e) {
        if (Input::IsMouseButtonPressed(MouseCode::ButtonRight)) {
            m_normal_camera_move_speed += e.GetYOffset() * 0.3f * m_normal_camera_move_speed;
            m_normal_camera_move_speed = glm::clamp(m_normal_camera_move_speed, min_camera_move_speed, max_camera_move_speed);
        } else {
            MouseZoom(e.GetYOffset() * 0.1f);
            UpdateCameraView();
        }
        return true;
    }

    void EditorCamera::MousePan(const glm::vec2& delta) {
        auto [x_speed, y_speed] = PanSpeed();
        m_focal_point -= GetRightDirection() * delta.x * x_speed * m_distance;
        m_focal_point += GetUpDirection() * delta.y * y_speed * m_distance;
    }

    void EditorCamera::MouseRotate(const glm::vec2& delta) {
        const float yaw_sign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
        m_yaw_delta += yaw_sign * delta.x * RotationSpeed();
        m_pitch_delta += delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(float delta) {
        m_distance -= delta * ZoomSpeed();
        const glm::vec3 forward = GetForwardDirection();
        m_position = m_focal_point - forward * m_distance;
        if (m_distance < 1.0f) {
            // Prevent flipping
            m_focal_point += forward * (1.0f - m_distance);
            m_distance = 1.0f;
        }
        // not sure
        m_position = m_focal_point - forward * m_distance;
        m_position_delta += forward * delta * ZoomSpeed();
    }

    glm::vec3 EditorCamera::GetUpDirection() const {
        return GetOrientation() * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 EditorCamera::GetRightDirection() const {
        return GetOrientation() * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 EditorCamera::GetForwardDirection() const {
        // return GetOrientation() * glm::vec3(0.0f, 0.0f, -1.0f);
        return GetOrientation() * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::quat EditorCamera::GetOrientation() const {
        return glm::quat(glm::vec3(-m_pitch - m_pitch_delta, -m_yaw - m_yaw_delta, 0.0f));
    }

    glm::vec3 EditorCamera::CalculatePosition() {
        // return m_focal_point - GetForwardDirection() * m_distance + m_position_delta;
        return m_focal_point - GetForwardDirection() * m_distance;
    }
}