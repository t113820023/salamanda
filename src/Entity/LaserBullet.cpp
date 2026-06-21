#include "Entity/LaserBullet.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"

LaserBullet::LaserBullet(float speed, int damage, float maxLifeTime, const glm::vec2& scale) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/laser.png"));
    m_Transform.scale = scale;
    SetZIndex(0.5f);
    m_Speed = speed;      // 雷射速度
    m_Damage = damage;    // 雷射傷害
    m_MaxLifeTime = maxLifeTime;  // 雷射飛行時間
}

const glm::vec2 LaserBullet::kDefaultScale = {3.0f, 3.0f};

void LaserBullet::Update() {
    float deltaTime = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_Transform.translation.x += m_Speed * deltaTime;
    m_LifeTime += deltaTime;
}
