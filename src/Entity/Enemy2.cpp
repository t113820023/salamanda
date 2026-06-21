#include "Entity/Enemy2.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"

Enemy2::Enemy2(glm::vec2 startPos) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/firebird_swangup.png"));
    m_Transform.translation = startPos;
    m_Transform.scale = {3.0f, 3.0f};
    SetZIndex(0.5f);
    LOG_INFO("Enemy2 created at ({}, {})", startPos.x, startPos.y);
}

void Enemy2::Update(const glm::vec2& characterPos) {
    const float deltaSeconds = Util::Time::GetDeltaTimeMs() / 1000.0f;

    if (!m_IsCharging) {
        m_IdleTimer += deltaSeconds;
        if (m_IdleTimer >= kIdleSeconds) {
            // 鎖定當下角色位置，計算固定衝刺方向（之後不再轉向）
            glm::vec2 direction = characterPos - m_Transform.translation;
            const float distance = glm::length(direction);
            if (distance > 0.0f) {
                direction = glm::normalize(direction);
            } else {
                direction = {-1.0f, 0.0f}; // 極端情況下（剛好重疊）退回預設向左飛行
            }
            m_ChargeDirection = direction;
            m_IsCharging = true;
            LOG_INFO("Enemy2 locked direction ({}, {}) towards character at ({}, {})",
                m_ChargeDirection.x, m_ChargeDirection.y, characterPos.x, characterPos.y);
        }
        // 待機階段：原地不動
        return;
    }

    // 衝刺階段：沿著鎖定的方向直線飛行
    m_Transform.translation += m_ChargeDirection * kChargeSpeed * deltaSeconds;
}