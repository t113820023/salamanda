#include "Entity/Fireball.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"

Fireball::Fireball(glm::vec2 startPos) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/fireball.png"));
    m_Transform.translation = startPos;
    m_Transform.scale = {1.0f, 1.0f};
    SetZIndex(0.5f);
    LOG_INFO("Fireball spawned at ({}, {})", startPos.x, startPos.y);
}

void Fireball::Update() {
    const float deltaSeconds = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_Transform.translation.x -= kSpeed * deltaSeconds; // 往左飛行（朝角色方向）
}