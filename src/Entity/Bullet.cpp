#include "Entity/Bullet.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"

Bullet::Bullet() {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/bullet.png"));
    m_Transform.scale = {3.0f, 3.0f};
    SetZIndex(0.5f);
}

void Bullet::Update() {
    float deltaTime = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_Transform.translation.x += m_Speed * deltaTime;
    m_LifeTime += deltaTime;
}