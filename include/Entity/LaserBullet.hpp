#ifndef LASER_BULLET_HPP
#define LASER_BULLET_HPP

#include "Bullet.hpp"
#include <glm/glm.hpp>

class LaserBullet : public Bullet {
public:
    static constexpr float kDefaultSpeed = 1000.0f;
    static constexpr int kDefaultDamage = 2;
    static constexpr float kDefaultMaxLifeTime = 3.0f;
    static constexpr int kBurstCount = 5;         // 連發數量
    static constexpr int kBurstDelay = 5;        // 連發間隔（幀數）
    static constexpr int kCooldown = 100;         // 連發完成後冷卻（幀數）
    static const glm::vec2 kDefaultScale;

    LaserBullet(float speed = kDefaultSpeed,
                int damage = kDefaultDamage,
                float maxLifeTime = kDefaultMaxLifeTime,
                const glm::vec2& scale = kDefaultScale);

    void Update() override;
};

#endif
