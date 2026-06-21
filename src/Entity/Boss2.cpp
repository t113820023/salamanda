#include "Entity/Boss2.hpp"
#include "Entity/Character.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <glm/glm.hpp>

Boss2::Boss2(glm::vec2 startPos) : m_Health(kHealth) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/BOSS2.png"));
    m_Transform.scale = {1.0f, 1.0f};
    SetZIndex(0.5f);

    // startPos.x 視為「期望的右緣 X 座標」（通常是螢幕右邊界），
    // 將實際中心位置往左偏移半個圖片寬度，使圖片右緣正好貼齊該座標。
    const float halfWidth = GetScaledSize().x * 0.5f;
    m_Transform.translation = {startPos.x - halfWidth, startPos.y};

    LOG_INFO("Boss2 spawned, right edge aligned at x={}, center at ({}, {})",
        startPos.x, m_Transform.translation.x, m_Transform.translation.y);
}

void Boss2::Update(const std::shared_ptr<Character>& character) {
    if (character) {
        // 只跟隨角色 Y 座標移動，X 座標永遠不變（保持右緣緊貼螢幕）
        m_Transform.translation.y = character->m_Transform.translation.y;
    }

    const float deltaSeconds = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_FireballTimer += deltaSeconds;
    if (m_FireballTimer >= kFireballInterval) {
        m_FireballTimer = 0.0f;
        m_HasPendingAttack = true;
    }
}

bool Boss2::IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth, float viewportHalfHeight) const {
    // Boss2 固定貼在螢幕右側，理論上不會離開畫面，此處維持基本範圍檢查作為保險。
    const float leftBound = cameraPos.x - viewportHalfWidth;
    const float rightBound = cameraPos.x + viewportHalfWidth;
    const float topBound = viewportHalfHeight;
    const float bottomBound = -viewportHalfHeight;

    return m_Transform.translation.x < leftBound ||
           m_Transform.translation.x > rightBound ||
           m_Transform.translation.y < bottomBound ||
           m_Transform.translation.y > topBound;
}