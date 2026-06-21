#include "Entity/Missile.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"

Missile::Missile(float speed, int damage, float maxLifeTime, const glm::vec2& scale) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/missile.png"));
    m_Transform.scale = scale;
    SetZIndex(0.5f);
    m_Speed = speed;      // 飛彈速度
    m_Damage = damage;    // 飛彈傷害
    m_MaxLifeTime = maxLifeTime;  // 飛彈飛行時間
}

const glm::vec2 Missile::kDefaultScale = {2.5f, 2.5f};

void Missile::Update() {
    float deltaTime = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_Transform.translation.x += m_Speed * deltaTime;
    m_LifeTime += deltaTime;
}
