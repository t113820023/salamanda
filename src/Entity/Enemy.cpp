#include "Entity/Enemy.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"

Enemy::Enemy(glm::vec2 startPos) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/enemy_1.png"));
    m_Transform.translation = startPos;
    m_Transform.scale = {3.0f, 3.0f};
    SetZIndex(0.5f);
    LOG_INFO("Enemy created at ({}, {})", startPos.x, startPos.y);
}

void Enemy::Update() {
    m_Transform.translation.x -= m_Speed * (Util::Time::GetDeltaTimeMs() / 1000.0f);
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) {  // 每60幀輸出一次
        // LOG_INFO("Enemy moving: ({}, {})", m_Transform.translation.x, m_Transform.translation.y);
    }
}