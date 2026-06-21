#ifndef SOUND_BULLET_HPP
#define SOUND_BULLET_HPP

#include "Bullet.hpp"
#include <glm/glm.hpp>

class SoundBullet : public Bullet {
public:
    static constexpr float kDefaultSpeed = 500.0f;
    static constexpr int kDefaultDamage = 1;
    static constexpr float kDefaultMaxLifeTime = 1.5f;
    static constexpr int kBurstCount = 4;         // 連發數量
    static constexpr int kBurstDelay = 12;        // 連發間隔（幀數）
    static constexpr int kCooldown = 100;         // 連發完成後冷卻（幀數）
    static const glm::vec2 kDefaultScale;

    SoundBullet(float speed = kDefaultSpeed,
                int damage = kDefaultDamage,
                float maxLifeTime = kDefaultMaxLifeTime,
                const glm::vec2& scale = kDefaultScale);

    void Update() override;
};

#endif
