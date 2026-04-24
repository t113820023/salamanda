#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Coordinate.hpp"
#include "..\PTSD\include\Core\Drawable.hpp" // 為了使用 Core::Matrices

namespace UGO {
namespace Graphics {
    
    class Camera {
    public:
        Camera(float scrollSpeed = 100.0f);

        Core::WorldPosition ScreenToWorld(const glm::vec2& screenPos) const;
        
        glm::vec2 WorldToScreen(const Core::WorldPosition& worldPos) const;
        
        void SetCameraPos(const Core::WorldPosition& cameraPos);
        Core::WorldPosition GetCameraPos() const;

        void UpdateBounds(const glm::vec2& imgSize) {
            // 左邊界：圖片左緣 + 半個視窗寬
            m_MinX = -(imgSize.x / 2.0f) + 128.0f;
            // 右邊界：圖片右緣 - 半個視窗寬
            m_MaxX = (imgSize.x / 2.0f) - 128.0f;
        }
        
        void Update(float deltaTime);

        // 取得當前相機生成的矩陣（View 與 Projection）
        // Model 矩陣會由個別物件自行提供
        ::Core::Matrices GetMatrices(const glm::mat4& modelMatrix) const;

        void SetPosition(const glm::vec2& pos) { m_cameraPos = pos; }
        glm::vec2 GetPosition() const { return m_cameraPos; }
        
    private:
        /* TODO: Add zoom functionality
        */
        float ZoomLevel = 1.0f;

        Core::WorldPosition m_cameraPos = {.0f, .0f};
        float m_ScrollSpeed;
        
        glm::mat4 m_ProjectionMatrix;

        float m_MinX = 0.0f;
        float m_MaxX = 0.0f;
        bool m_UseBounds = true;
    };

} // namespace Graphics
} // namespace UGO

#endif // CAMERA_HPP