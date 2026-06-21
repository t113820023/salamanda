#include "Entity/Boss1.hpp"
#include "Entity/Character.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <glm/glm.hpp>

Boss1::Boss1(glm::vec2 startPos) : m_Health(kHealth) {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/Boss1_inc.png"));
    m_Transform.translation = startPos;
    m_Transform.scale = {3.0f, 3.0f};
    SetZIndex(0.5f);
    LOG_INFO("Boss1 spawned at ({}, {})", startPos.x, startPos.y);
}

void Boss1::Update(const std::shared_ptr<Character>& character) {
    if (!character) return;

    // 計算朝向角色的方向
    glm::vec2 directionToCharacter = character->m_Transform.translation - m_Transform.translation;
    
    // 避免向量長度為 0
    float distance = glm::length(directionToCharacter);
    if (distance > 0.0f) {
        directionToCharacter = glm::normalize(directionToCharacter);
    }

    // 朝向角色移動
    float deltaTime = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_Transform.translation += directionToCharacter * kSpeed * deltaTime;

    static int frameCount = 0;
    frameCount++;
    if (frameCount % 120 == 0) {  // 每 120 幀輸出一次
        LOG_INFO("Boss1 moving towards character at ({}, {}), Health: {}", 
            m_Transform.translation.x, m_Transform.translation.y, m_Health);
    }
}

bool Boss1::IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth, float viewportHalfHeight) const {
    const float leftBound = cameraPos.x - viewportHalfWidth;
    const float rightBound = cameraPos.x + viewportHalfWidth;
    const float topBound = viewportHalfHeight;
    const float bottomBound = -viewportHalfHeight;

    return m_Transform.translation.x < leftBound ||
           m_Transform.translation.x > rightBound ||
           m_Transform.translation.y < bottomBound ||
           m_Transform.translation.y > topBound;
}