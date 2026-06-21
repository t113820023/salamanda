#ifndef STAGE_1_1_HPP
#define STAGE_1_1_HPP

#include "Stage.hpp"
#include "Entity/Boss1.hpp"
#include <glm/glm.hpp>

class App;

/**
 * @class Stage1_1
 * @brief Implements the main game level "1-1" (from Enter until Boss1 defeat).
 */
class Stage1_1 : public Stage {
public:
    explicit Stage1_1(App* app);
    ~Stage1_1() override = default;

    void Enter() override;
    void Update(float deltaSeconds) override;
    void Exit() override;
    bool IsStageComplete() const override;
    std::string GetStageName() const override;
    std::string GetNextStageName() const override { return "1-2"; }

private:
    void StartEnemyBurst();
    int GetRandomEnemyBurstInterval() const;
    void CheckAndSpawnBoss(); // 檢查分數/時間條件，決定是否生成 Boss1

    App* m_App;
    bool m_IsStageComplete = false;

    // --- 波次控制狀態 ---
    int m_EnemyBurstCount = 0;                   
    int m_EnemyBurstSpawnCounter = 0;         
    int m_EnemyBurstInterval = 0;             
    bool m_IsEnemyBurstActive = false;        
    bool m_IsWaitingForEnemyWave = false;     
    int m_EnemyWaveWaitCounter = 0;           
    glm::vec2 m_EnemyBurstSpawnPosition = glm::vec2{0.0f, 0.0f}; 
};

#endif