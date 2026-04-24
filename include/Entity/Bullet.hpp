#ifndef BULLET_HPP
#define BULLET_HPP

#include "Util/GameObject.hpp"
#include <glm/glm.hpp>

class Bullet : public Util::GameObject {
public:
    Bullet();
    virtual ~Bullet() = default;
    virtual void Update();
    bool IsOffScreen(const glm::vec2& cameraPos) const {
        // 投影視野範圍：水平 ±128，垂直 ±128
        const float leftBound = cameraPos.x - 128.0f;
        const float rightBound = cameraPos.x + 128.0f;
        const float topBound = 128.0f;
        const float bottomBound = -128.0f;

        return m_Transform.translation.x < leftBound ||
               m_Transform.translation.x > rightBound ||
               m_Transform.translation.y < bottomBound ||
               m_Transform.translation.y > topBound;
    }

    int GetDamage() const { return m_Damage; }
    bool IsOutOfLife() const { return m_LifeTime > m_MaxLifeTime; }

protected:
    float m_Speed = 200.0f;
    int m_Damage = 1;  // 子彈傷害值
    float m_LifeTime = 0.0f;        // 飛行時間（秒）
    float m_MaxLifeTime = 0.5f;    // 最大飛行時間（秒）
};

#endif