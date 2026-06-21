#ifndef ENEMY2_HPP
#define ENEMY2_HPP

#include "Entity/EnemyBase.hpp"
#include <glm/glm.hpp>

/**
 * @class Enemy2
 * @brief 新增敵人：生成後待在原地 3 秒，3 秒後鎖定當時角色所在位置，
 *        計算出一個固定方向，之後沿著該方向直線飛行（不持續追蹤）。
 */
class Enemy2 : public EnemyBase {
public:
    static constexpr int kHealth = 1;            // 生命值，與一般 Enemy 相同
    static constexpr float kChargeSpeed = 400.0f; // 鎖定方向後的衝刺速度
    static constexpr float kIdleSeconds = 3.0f;   // 生成後待在原地的秒數

    explicit Enemy2(glm::vec2 startPos);

    void Update(const glm::vec2& characterPos) override;

    bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const override {
        const float leftBound = cameraPos.x - viewportHalfWidth;
        const float rightBound = cameraPos.x + viewportHalfWidth;
        const float topBound = viewportHalfHeight;
        const float bottomBound = -viewportHalfHeight;

        // 衝刺方向不一定向左，因此四個邊都要檢查（一般 Enemy 只往左飛，不需要檢查右邊界）
        return m_Transform.translation.x < leftBound ||
               m_Transform.translation.x > rightBound ||
               m_Transform.translation.y < bottomBound ||
               m_Transform.translation.y > topBound;
    }

    int GetHealth() const override { return m_Health; }
    void TakeDamage(int damage) override { m_Health -= damage; }
    bool IsDead() const override { return m_Health <= 0; }

private:
    int m_Health = kHealth;
    float m_IdleTimer = 0.0f;          // 已待機的秒數
    bool m_IsCharging = false;         // 是否已鎖定方向進入衝刺階段
    glm::vec2 m_ChargeDirection = {0.0f, 0.0f}; // 鎖定後的衝刺方向（單位向量）
};

#endif // ENEMY2_HPP