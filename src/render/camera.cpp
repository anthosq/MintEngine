#include "render/camera.h"

namespace Mint {
    Camera::Camera(const float deg_fov, const float aspect_ratio, const float near_clip, const float far_clip) {
        m_projection_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, far_clip , near_clip);
        m_unreserved_z_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, near_clip, far_clip);
    }
}