#ifndef SOUND_BULLET_HPP
#define SOUND_BULLET_HPP

#include "Bullet.hpp"
#include <glm/glm.hpp>

class SoundBullet : public Bullet {
public:
    static constexpr float kDefaultSpeed = 150.0f;
    static constexpr int kDefaultDamage = 1;
    static constexpr float kDefaultMaxLifeTime = 1.5f;
    static const glm::vec2 kDefaultScale;

    SoundBullet(float speed = kDefaultSpeed,
                int damage = kDefaultDamage,
                float maxLifeTime = kDefaultMaxLifeTime,
                const glm::vec2& scale = kDefaultScale);

    void Update() override;
};

#endif
