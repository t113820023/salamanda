#ifndef MISSILE_HPP
#define MISSILE_HPP

#include "Bullet.hpp"
#include <glm/glm.hpp>

class Missile : public Bullet {
public:
    static constexpr float kDefaultSpeed = 400.0f;
    static constexpr int kDefaultDamage = 3;
    static constexpr float kDefaultMaxLifeTime = 3.0f;
    static const glm::vec2 kDefaultScale;

    Missile(float speed = kDefaultSpeed,
            int damage = kDefaultDamage,
            float maxLifeTime = kDefaultMaxLifeTime,
            const glm::vec2& scale = kDefaultScale);

    void Update() override;
};

#endif
