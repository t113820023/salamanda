#include "Entity/Enemy.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"

// 敵人配置plate的默認實例
static const EnemyPlate g_DefaultEnemyPlate;

const EnemyPlate& Enemy::GetPlate() {
    return g_DefaultEnemyPlate;
}

Enemy::Enemy(glm::vec2 startPos) {
    const EnemyPlate& plate = GetPlate();
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + plate.imagePath));
    m_Transform.translation = startPos;
    m_Transform.scale = plate.scale;
    SetZIndex(plate.zIndex);
    m_Health = plate.health;
    m_Speed = plate.speed;
    LOG_INFO("Enemy created at ({}, {})", startPos.x, startPos.y);
}

void Enemy::Update(const glm::vec2& characterPos) {
    (void)characterPos; 
    m_Transform.translation.x -= m_Speed * (Util::Time::GetDeltaTimeMs() / 1000.0f);
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) {
        // LOG_INFO("Enemy moving: ({}, {})", m_Transform.translation.x, m_Transform.translation.y);
    }
}