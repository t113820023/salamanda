#ifndef BULLET_HPP
#define BULLET_HPP

#include "Util/GameObject.hpp"
#include <glm/glm.hpp>

class Bullet : public Util::GameObject {
public:
    Bullet();
    static constexpr float kDefaultSpeed = 600.0f;
    static constexpr int kDefaultDamage = 1;
    static constexpr float kDefaultMaxLifeTime = 3.0f;
    static constexpr int kBurstCount = 3;         // 連發數量
    static constexpr int kBurstDelay = 10;        // 連發間隔（幀數）
    static constexpr int kCooldown = 90;         // 連發完成後冷卻（幀數）
    virtual ~Bullet() = default;
    virtual void Update();
    bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const {
        // 投影視野範圍：動態設定
        const float leftBound = cameraPos.x - viewportHalfWidth;
        const float rightBound = cameraPos.x + viewportHalfWidth;
        const float topBound = viewportHalfHeight;
        const float bottomBound = -viewportHalfHeight;

        return m_Transform.translation.x < leftBound ||
               m_Transform.translation.x > rightBound ||
               m_Transform.translation.y < bottomBound ||
               m_Transform.translation.y > topBound;
    }

    int GetDamage() const { return m_Damage; }
    bool IsOutOfLife() const { return m_LifeTime > m_MaxLifeTime; }

protected:
    float m_Speed = 600.0f;
    int m_Damage = 1;  // 子彈傷害值
    float m_LifeTime = 0.0f;        // 飛行時間（秒）
    float m_MaxLifeTime = 3.0f;    // 最大飛行時間（秒）
};

#endif