#include "stages/Stage1_1.hpp"
#include "App.hpp"
#include "Entity/Enemy.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include <random>

Stage1_1::Stage1_1(App* app) 
    : m_App(app), m_IsStageComplete(false) {
}

void Stage1_1::Enter() {
    LOG_INFO("Stage 1-1: Entering");
    
    // 切換背景
    m_App->SetBackgroundImage("/Image/1-1.png");
    
    // 生成主角
    m_App->SpawnCharacter(2);
    
    // 取得怪物波次設定
    const EnemyPlate& enemyPlate = Enemy::GetPlate();
    LOG_INFO("Stage 1-1: Enemy configuration - Wave delay: {}, Burst size: {}, Burst interval: {}-{}", 
             enemyPlate.waveInitialDelay, enemyPlate.burstSize, 
             enemyPlate.burstIntervalMin, enemyPlate.burstIntervalMax);
    
    // 初始化波次狀態
    m_IsStageComplete = false;
    m_IsWaitingForEnemyWave = true;
    m_EnemyWaveWaitCounter = 0;
    m_IsEnemyBurstActive = false;
}

void Stage1_1::Update(float deltaSeconds) {
    (void)deltaSeconds; 

    const EnemyPlate& enemyPlate = Enemy::GetPlate();

    // 1. 處理開場等待時間
    if (m_IsWaitingForEnemyWave) {
        m_EnemyWaveWaitCounter++;
        if (m_EnemyWaveWaitCounter >= enemyPlate.waveInitialDelay) {
            m_IsWaitingForEnemyWave = false;
            StartEnemyBurst();
        }
    }

    // 2. 處理敵人連發 (Burst)
    if (m_IsEnemyBurstActive) {
        if (m_EnemyBurstCount < enemyPlate.burstSize) {
            m_EnemyBurstSpawnCounter++;
            if (m_EnemyBurstSpawnCounter >= enemyPlate.burstSpawnDelay) {
                // 呼叫 App 在指定位置生成敵人
                m_App->SpawnEnemyAt(m_EnemyBurstSpawnPosition);
                m_EnemyBurstCount++;
                m_EnemyBurstSpawnCounter = 0;
            }
        }

        // 這波出完了
        if (m_EnemyBurstCount >= enemyPlate.burstSize) {
            m_IsEnemyBurstActive = false;
            m_EnemyBurstInterval = GetRandomEnemyBurstInterval();
            m_EnemyBurstSpawnCounter = 0;
            LOG_INFO("Enemy burst complete. Next burst in {} frames", m_EnemyBurstInterval);
        }
    } else {
        // 等待下一波
        if (!m_IsWaitingForEnemyWave) {
            m_EnemyBurstSpawnCounter++;
            if (m_EnemyBurstSpawnCounter >= m_EnemyBurstInterval) {
                StartEnemyBurst();
            }
        }
    }

    // 3. 檢查過關條件
    if (m_App->IsBoss1Defeated()) {
        m_IsStageComplete = true;
        LOG_INFO("Stage 1-1: Boss1 defeated! Stage complete!");
    }
}

void Stage1_1::Exit() {
    LOG_INFO("Stage 1-1: Exiting");
}

bool Stage1_1::IsStageComplete() const {
    return m_IsStageComplete;
}

std::string Stage1_1::GetStageName() const {
    return "1-1";
}

// 實作波次開始的邏輯
void Stage1_1::StartEnemyBurst() {
    m_IsEnemyBurstActive = true;
    m_EnemyBurstCount = 0;
    m_EnemyBurstSpawnCounter = 0;
    // 從 App 取得隨機生成點
    m_EnemyBurstSpawnPosition = m_App->GetRandomEnemySpawnPosition();
    LOG_INFO("Starting new enemy burst at ({}, {})", m_EnemyBurstSpawnPosition.x, m_EnemyBurstSpawnPosition.y);
}

// 實作隨機間隔的邏輯
int Stage1_1::GetRandomEnemyBurstInterval() const {
    const EnemyPlate& enemyPlate = Enemy::GetPlate();
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(enemyPlate.burstIntervalMin, enemyPlate.burstIntervalMax);
    return dis(gen);
}