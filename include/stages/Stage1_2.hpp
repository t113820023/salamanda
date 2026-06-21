#ifndef STAGE_1_2_HPP
#define STAGE_1_2_HPP

#include "Stage.hpp"
#include "Entity/Boss2.hpp"
#include <glm/glm.hpp>

class App;

/**
 * @class Stage1_2
 * @brief Implements the main game level "1-2" (from Enter until Boss2 defeat).
 *
 * 與 Stage1_1 共用相同的一般敵人波次出怪機制與 Boss 生成判斷邏輯，
 * 額外新增 Enemy2 的單一生成、隨機間隔機制。
 * 擊敗 Boss2 後不會直接通關，而是呼叫 App::EnterEndingScreen() 進入
 * 結算階段，IsStageComplete() 同時回傳 true 讓本關卡結束。
 */
class Stage1_2 : public Stage {
public:
    explicit Stage1_2(App* app);
    ~Stage1_2() override = default;

    void Enter() override;
    void Update(float deltaSeconds) override;
    void Exit() override;
    bool IsStageComplete() const override;
    std::string GetStageName() const override;

private:
    void StartEnemyBurst();
    int GetRandomEnemyBurstInterval() const;
    void CheckAndSpawnBoss(); // 檢查分數/時間條件，決定是否生成 Boss2
    void UpdateEnemy2Spawning(); // Enemy2 單一生成、隨機間隔邏輯
    int GetRandomEnemy2SpawnInterval() const;

    App* m_App;
    bool m_IsStageComplete = false;

    // --- 一般敵人波次控制狀態（與 Stage1_1 相同）---
    int m_EnemyBurstCount = 0;
    int m_EnemyBurstSpawnCounter = 0;
    int m_EnemyBurstInterval = 0;
    bool m_IsEnemyBurstActive = false;
    bool m_IsWaitingForEnemyWave = false;
    int m_EnemyWaveWaitCounter = 0;
    glm::vec2 m_EnemyBurstSpawnPosition = glm::vec2{0.0f, 0.0f};

    // --- Enemy2 生成控制狀態（單一生成、隨機間隔 120~240 幀）---
    static constexpr int kEnemy2SpawnIntervalMin = 120;
    static constexpr int kEnemy2SpawnIntervalMax = 240;
    int m_Enemy2SpawnCounter = 0;
    int m_Enemy2SpawnInterval = 0;
};

#endif