#ifndef LASER_BULLET_HPP
#define LASER_BULLET_HPP

#include "Bullet.hpp"
#include <glm/glm.hpp>

class LaserBullet : public Bullet {
public:
    static constexpr float kDefaultSpeed = 300.0f;
    static constexpr int kDefaultDamage = 2;
    static constexpr float kDefaultMaxLifeTime = 0.8f;
    static const glm::vec2 kDefaultScale;

    LaserBullet(float speed = kDefaultSpeed,
                int damage = kDefaultDamage,
                float maxLifeTime = kDefaultMaxLifeTime,
                const glm::vec2& scale = kDefaultScale);

    void Update() override;
};

#endif
