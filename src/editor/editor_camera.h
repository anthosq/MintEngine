#pragma once
#include "render/camera.h"
#include "core/time_step.h"
#include "event/key_event.h"
#include "event/mouse_event.h"


namespace Mint {
    // temporary CameraMode 
    enum class CameraMode {
        None = 0,
        FLYCAM = 1,
        ARCBALL = 2
    };



    class EditorCamera : public Camera {
        // handle movement input from editor
        // need to own view and model matrix

        // camera与editor camera分开负责Projection与View矩阵的计算与储存
        public:
            EditorCamera(float degFov, float aspectRatio, float nearClip, float farClip);
            void Init();

            void Focus(const glm::vec3& focusPoint);
            void OnUpdate(TimeStep ts);
            void OnEvent(Event& e);

            CameraMode GetCameraMode() const { return m_camera_mode; }

            inline float GetDistance() const { return m_distance; }
            inline void SetDistance(float distance) { m_distance = distance; }

            // target point
            const glm::vec3& GetFocalPoint() const { return m_focal_point; }

            // viewport
            // not sure, maybe editor camera should know viewport size
            void SetViewportSize(float width, float height) {
                m_aspect_ratio = width / height;
                SetPerspectiveProjection(m_vertical_fov, m_aspect_ratio, m_nearClip, m_farClip);

            }

            bool IsActive() const { return m_active; }
            void SetActive(bool active) { m_active = active; }

            glm::mat4 GetViewMatrix() const { return m_viewmatrix; }
            glm::mat4 GetViewProjection() const { return GetProjectionMatrix() * m_viewmatrix; }
            glm::mat4 GetUnreservedViewProjection() const { return GetUnreservedZMatrix() * m_viewmatrix; }

            glm::vec3 GetUpDirection() const;
            glm::vec3 GetRightDirection() const;
            glm::vec3 GetForwardDirection() const;

            const glm::vec3 GetPosition() const { return m_position; }

            glm::quat GetOrientation() const;


            float GetPitch() const { return m_pitch; }
            float GetYaw() const { return m_yaw; }
            float GetNearClip() const { return m_nearClip; }
            float GetFarClip() const { return m_farClip; }
            float GetFov() const { return m_vertical_fov; }
            float GetCameraMoveSpeed() const;
            float GetAspectRatio() const { return m_aspect_ratio; }

        private:
            void UpdateCameraView();

            bool OnMouseScroll(MouseScrolledEvent& e);

            void MousePan(const glm::vec2& delta);
            void MouseRotate(const glm::vec2& delta);
            void MouseZoom(float delta);


            glm::vec3 CalculatePosition();
            std::pair<float, float> PanSpeed() const;
            float RotationSpeed() const;
            float ZoomSpeed() const;
            

        private:
            // !TODO: Maybe future the camera should know the bounds of the viewport, for casting rays from mouse position

            // editor camera own view matrix
            glm::mat4 m_viewmatrix;
            glm::vec3 m_position, m_direction, m_focal_point;

            // projection parameters
            float m_vertical_fov, m_aspect_ratio, m_nearClip, m_farClip;

            float m_distance = 10.0f;
            CameraMode m_camera_mode = CameraMode::ARCBALL;

            // Euler Angles
            float m_pitch = 0.0f;
            float m_yaw = 0.0f;
            float m_pitch_delta = 0.0f;
            float m_yaw_delta = 0.0f;


            bool m_active = true;
            bool m_panning, m_rotating;

            glm::vec2 m_last_mouse_position {};
            glm::vec3 m_last_focal_point, m_last_rotation;

            glm::vec3 m_position_delta {};
            glm::vec3 m_right_direction {};

            float m_min_focus_distance = 100.0f;

            float m_viewport_width = 1280.0f, m_viewport_height = 720.0f;

            constexpr static float min_camera_move_speed = 0.005f, max_camera_move_speed = 2.0f;
            // temporary
            float m_normal_camera_move_speed = 0.1f;
    };
}