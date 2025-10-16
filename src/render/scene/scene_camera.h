#pragma once
#include "render/camera.h"
#include <glm/glm.hpp>
#include <tuple>

namespace Mint {
    class SceneCamera : public Camera{
    public:
        // 确定投影类型 
        enum class ProjectionType { Orthographic = 0, Perspective = 1 };

        // Set
        void SetOrthographic(float size, float aspect_ratio, float oNear, float oFar);

        void SetPerspective(float fov_deg, float aspect_ratio, float pNear, float pFar);

        void SetViewportBounds(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

        void SetDegPerspectiveFov(const float deg_fov) { m_perspective_fov_deg = deg_fov; }
        void SetPerspectiveNearClip(const float pNear) { m_perspective_near = pNear; }
        void SetPerspectiveFarClip(const float pFar) { m_perspective_far = pFar; }

        void SetOrthoSize(const float size) { m_ortho_size = size; }
        void SetOrthoNearClip(const float oNear) { m_ortho_near = oNear; }
        void SetOrthoFarClip(const float oFar) { m_ortho_far = oFar; }

        void SetProjectionType(ProjectionType type) { m_projection_type = type; }

        // Get
        ProjectionType GetProjectionType() const { return m_projection_type; }
        std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetViewportBounds() const { return m_viewport_bounds; }
        
        float GetPerspectiveFov() const { return m_perspective_fov_deg; }
        float GetPerspectiveNearClip() const { return m_perspective_near; }
        float GetPerspectiveFarClip() const { return m_perspective_far; }

        float GetOrthoSize() const { return m_ortho_size; }
        float GetOrthoNearClip() const { return m_ortho_near; }
        float GetOrthoFarClip() const { return m_ortho_far; }


    private:
        float m_perspective_fov_deg = 45.0f;
        float m_perspective_near = 0.1f, m_perspective_far = 1000.0f;

        float m_ortho_size = 10.0f;
        float m_ortho_near = -1.0f, m_ortho_far = 1.0f;

        ProjectionType m_projection_type = ProjectionType::Perspective;
        std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> m_viewport_bounds = {0, 0, 0, 0};
    
    };
}