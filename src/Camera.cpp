#include "Camera.hpp"

#include <algorithm>

namespace UGO {
namespace Graphics {
    Camera::Camera(float windowWidth, float windowHeight, float scrollSpeed) 
        : m_ScrollSpeed(scrollSpeed) {
        m_ViewportHalfWidth = windowWidth / 2.0f;
        m_ViewportHalfHeight = windowHeight / 2.0f;
        m_ProjectionMatrix = glm::ortho(-m_ViewportHalfWidth, m_ViewportHalfWidth, 
                                        -m_ViewportHalfHeight, m_ViewportHalfHeight, -2.0f, 2.0f);
    }

    UGO::Core::WorldPosition Camera::ScreenToWorld(const glm::vec2& screenPos) const {
        return screenPos + m_cameraPos;
    }

    glm::vec2 Camera::WorldToScreen(const UGO::Core::WorldPosition& worldPos) const {
        return worldPos - m_cameraPos;
    }

    void Camera::SetCameraPos(const UGO::Core::WorldPosition& cameraPos) {
        /* TODO: Check if the position is valid
        */
        m_cameraPos = cameraPos;
    }

    UGO::Core::WorldPosition Camera::GetCameraPos() const {
        return m_cameraPos;
    }

    ::Core::Matrices Camera::GetMatrices(const glm::mat4& modelMatrix) const {
        // 1. 計算 View 矩陣（根據相機當前位置 m_cameraPos）
        glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-m_cameraPos, 0.0f));

    }

    void Camera::Update(float deltaTime) {
        // 1. 正常的位移邏輯
        m_cameraPos.x += m_ScrollSpeed * deltaTime;
  
        m_cameraPos.x = std::clamp(m_cameraPos.x, m_MinX, m_MaxX);
    }
} // namespace Graphics
} // namespace UGO