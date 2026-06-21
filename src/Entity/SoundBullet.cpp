#include "Entity/SoundBullet.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"

SoundBullet::SoundBullet(float speed, int damage, float maxLifeTime, const glm::vec2& scale) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/sound.png"));
    m_Transform.scale = scale;  // 聲波尺寸
    SetZIndex(0.5f);
    m_Speed = speed;      // 聲波速度
    m_Damage = damage;    // 聲波傷害
    m_MaxLifeTime = maxLifeTime;  // 聲波飛行時間
}

const glm::vec2 SoundBullet::kDefaultScale = {2.0f, 2.0f};

void SoundBullet::Update() {
    float deltaTime = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_Transform.translation.x += m_Speed * deltaTime;
    m_LifeTime += deltaTime;
}
