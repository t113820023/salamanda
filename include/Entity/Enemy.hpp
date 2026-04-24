#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Util/GameObject.hpp"
#include <glm/glm.hpp>

class Enemy : public Util::GameObject {
public:
    Enemy(glm::vec2 startPos);
    void Update();
    bool IsOffScreen(const glm::vec2& cameraPos) const {
        // 投影寬度是256（-128到128），左側邊界 = 相機位置 - 128
        return m_Transform.translation.x < cameraPos.x - 128.0f;
    }

    int GetHealth() const { return m_Health; }
    void TakeDamage(int damage) { m_Health -= damage; }
    bool IsDead() const { return m_Health <= 0; }

private:
    int m_Health = 2;  // 敵人生命值
    float m_Speed = 40.0f;  // 敵人移動速度（向左）
};

#endif