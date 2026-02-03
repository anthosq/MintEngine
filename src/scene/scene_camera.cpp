#include "scene_camera.h"

namespace Mint {

    void SceneCamera::SetOrthographic(float size, float aspect_ratio, float oNear, float oFar) {
        m_projection_type = ProjectionType::Orthographic;
        m_ortho_size = size;
        m_ortho_near = oNear;
        m_ortho_far = oFar;
    }

    void SceneCamera::SetPerspective(float fov_deg, float aspect_ratio, float pNear, float pFar) {
        m_projection_type = ProjectionType::Perspective;
        m_perspective_fov_deg = fov_deg;
        m_perspective_near = pNear;
        m_perspective_far = pFar;
    }

    void SceneCamera::SetViewportBounds(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) {
        m_viewport_bounds = {left, top, right, bottom};
        float width = (float)(right - left);
        float height = (float)(bottom - top);
        float aspect_ratio = width / height;
    
        switch(m_projection_type) {
            case ProjectionType::Orthographic: {
                SetOrthoProjectionMatrix(m_ortho_size * aspect_ratio, m_ortho_size, m_ortho_near, m_ortho_far);
                break;
            }
            case ProjectionType::Perspective: {
                SetPerspectiveProjection(m_perspective_fov_deg, aspect_ratio, m_perspective_near, m_perspective_far);
                break;
            }

        }
    }
}