#include "render/camera.h"

namespace Mint {
    // Camera::Camera(const float deg_fov, const float aspect_ratio, const float near_clip, const float far_clip) {
    //     m_projection_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, far_clip , near_clip);
    //     m_unreserved_z_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, near_clip, far_clip);
    // }

    // temporary functions, later move to editor camera
    void Camera::RecalculateViewMatrix() {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) *
                              glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0, 0, 1));
        m_view_matrix = glm::inverse(transform);
    }
}