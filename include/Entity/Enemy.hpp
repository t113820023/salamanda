#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Entity/EnemyBase.hpp"
#include <glm/glm.hpp>
#include <string>

/**
 * @class EnemyPlate
 * @brief 敵人配置信息集合，包含敵人的所有設定數值
 */
class EnemyPlate {
public:
    // 波次生成相關配置
    int waveInitialDelay = 30;      // 開始遊戲後等待第一波生成的幀數
    int burstSpawnDelay = 40;       // 一波內每隻敵人連續生成的間隔幀數
    int burstSize = 5;               // 一波內的敵人數量
    int burstIntervalMin = 120;      // 每波之間最短等待幀數
    int burstIntervalMax = 240;      // 每波之間最長等待幀數

    // 個體敵人配置
    int health = 1;                  // 敵人生命值
    float speed = 200.0f;            // 敵人移動速度（向左）
    std::string imagePath = "/Image/enemy_1.png";  // 敵人圖片路徑
    glm::vec2 scale = {3.0f, 3.0f}; // 敵人縮放
    float zIndex = 0.5f;             // Z-index
};

class Enemy : public EnemyBase {
public:
    // 敵人生成相關參數
    static constexpr int kWaveInitialDelay = 30;      // 開始遊戲後等待第一波生成的幀數
    static constexpr int kBurstSpawnDelay = 40;      // 一波內每隻敵人連續生成的間隔幀數
    static constexpr int kBurstSize = 5;              // 一波內的敵人數量
    static constexpr int kBurstIntervalMin = 120;     // 每波之間最短等待幀數
    static constexpr int kBurstIntervalMax = 240;     // 每波之間最長等待幀數

    // 獲取敵人配置plate
    static const EnemyPlate& GetPlate();

    Enemy(glm::vec2 startPos);

    void Update(const glm::vec2& characterPos) override;

    bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const override {
        const float leftBound = cameraPos.x - viewportHalfWidth;
        const float topBound = viewportHalfHeight;
        const float bottomBound = -viewportHalfHeight;

        return m_Transform.translation.x < leftBound ||
               m_Transform.translation.y < bottomBound ||
               m_Transform.translation.y > topBound;
    }

    int GetHealth() const override { return m_Health; }
    void TakeDamage(int damage) override { m_Health -= damage; }
    bool IsDead() const override { return m_Health <= 0; }

private:
    int m_Health;     // 敵人生命值（建構子依 EnemyPlate::health 初始化）
    float m_Speed;    // 敵人移動速度（建構子依 EnemyPlate::speed 初始化）
};

#endif