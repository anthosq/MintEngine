#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Mint {
    // physical camera and virtual camera in future

    // base class of camera
    // 职责分离, SceneCamera提供view矩阵, camera基类提供投影矩阵
    // SceneCamera通过SetOrthographic, SetPerspective等方法设置投影矩阵
    // EditorCamera提供交互功能, 
    class Camera {
    public:
        Camera() = default;
        Camera(const glm::mat4& projection, const glm::mat4& unreserved_z)
            : m_projection_matrix(projection), m_unreserved_z_matrix(unreserved_z) {}
        // Camera(const float deg_fov, const float aspect_ratio, const float zNear, const float zFar);
        virtual ~Camera() = default;

        glm::mat4 GetProjectionMatrix() const { return m_projection_matrix; }
        glm::mat4 GetUnreservedZMatrix() const { return m_unreserved_z_matrix; }

        void SetProjectionMatrix(const glm::mat4& projection, const glm::mat4& unreserved_z) {
            m_projection_matrix = projection; 
            m_unreserved_z_matrix = unreserved_z;
        }

        void SetPerspectiveProjection(float deg_fov, float aspect_ratio, float zNear, float zFar) {
            m_projection_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, zFar , zNear);
            m_unreserved_z_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, zNear, zFar);
        }

        void SetPerspectiveProjectionRad(float rad_fov, float aspect_ratio, float zNear, float zFar) {
            m_projection_matrix = glm::perspective(rad_fov, aspect_ratio, zFar , zNear);
            m_unreserved_z_matrix = glm::perspective(rad_fov, aspect_ratio, zNear, zFar);
        }

        void SetOrthoProjectionMatrix(const float width, const float height, const float zNear, const float zFar) {
            m_projection_matrix = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, zFar, zNear);
            m_unreserved_z_matrix = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, zNear, zFar);
        }

        // not sure
        float GetExposure() const { return m_exposure; }
        

        // temporary functinos, later move to editor camera
        const glm::vec3 GetPosition() const { return m_position; }
        void SetPosition(const glm::vec3& position) { m_position = position; RecalculateViewMatrix(); }

        float GetRotation() const { return m_rotation; }
        void SetRotation_deg(const float rotation) { m_rotation = rotation; RecalculateViewMatrix(); }
        // void SetRotation_rad(const float rotation) { m_rotation = rotation; }
        const glm::mat4& GetViewMatrix() const { return m_view_matrix; }
        Camera(float left, float right, float bottom, float top) 
        {
            m_projection_matrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
            m_unreserved_z_matrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        }
        // --------------------------------------------------

    protected:
        float m_exposure = 0.8f;

    private:
        void RecalculateViewMatrix();
        glm::mat4 m_projection_matrix = glm::mat4(1.0f);
        // UnReserved Z
        glm::mat4 m_unreserved_z_matrix = glm::mat4(1.0f); 


        // temporary variables & functions, later move to editor camera
        glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
        glm::mat4 m_view_matrix = glm::mat4(1.0f);
        float m_rotation = 0.0f;
    };
}