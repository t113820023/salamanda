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
    
    // Switch background to 1-1 map
    m_App->SetBackgroundImage("/Image/1-1.png");

    // 設定本關卡使用的地圖橫幅
    m_App->SetBannerPaths("/Image/1-1地圖橫幅(上).png", "/Image/1-1地圖橫幅(下).png");
    
    // Spawn player character with starting health
    m_App->SpawnCharacter(2);

    // 顯示底部 HUD（生命值/分數/強化提示條）
    m_App->ShowHud();
    
    // Get enemy wave configuration from EnemyPlate
    const EnemyPlate& enemyPlate = Enemy::GetPlate();
    LOG_INFO("Stage 1-1: Enemy configuration - Wave delay: {}, Burst size: {}, Burst interval: {}-{}", 
             enemyPlate.waveInitialDelay, enemyPlate.burstSize, 
             enemyPlate.burstIntervalMin, enemyPlate.burstIntervalMax);
    
    // Reset stage completion flag
    m_IsStageComplete = false;
    m_IsWaitingForEnemyWave = true;
    m_EnemyWaveWaitCounter = 0;
    m_IsEnemyBurstActive = false;
}

void Stage1_1::Update(float deltaSeconds) {
    (void)deltaSeconds;  

    // Boss 戰鬥已開始時，暫停一般敵人出怪
    if (!m_App->IsBossActive()) {
        const EnemyPlate& enemyPlate = Enemy::GetPlate();

        if (m_IsWaitingForEnemyWave) {
            m_EnemyWaveWaitCounter++;
            if (m_EnemyWaveWaitCounter >= enemyPlate.waveInitialDelay) {
                m_IsWaitingForEnemyWave = false;
                StartEnemyBurst();
            }
        }

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
    }

    // 由本關卡自行檢查 Boss 生成條件（分數達門檻，或關卡持續時間達 5 分鐘）
    CheckAndSpawnBoss();

    if (m_App->IsBossDefeated()) {
        m_IsStageComplete = true;
        LOG_INFO("Stage 1-1: Boss defeated! Stage complete!");
    }
}

void Stage1_1::Exit() {
    LOG_INFO("Stage 1-1: Exiting");
    m_App->HideHud();
    m_App->ClearBannerPaths();
    // Cleanup will be handled by App or next stage
}

bool Stage1_1::IsStageComplete() const {
    return m_IsStageComplete;
}

std::string Stage1_1::GetStageName() const {
    return "1-1";
}

void Stage1_1::StartEnemyBurst() {
    m_IsEnemyBurstActive = true;
    m_EnemyBurstCount = 0;
    m_EnemyBurstSpawnCounter = 0;
    // 從 App 取得隨機生成點
    m_EnemyBurstSpawnPosition = m_App->GetRandomEnemySpawnPosition();
    LOG_INFO("Starting new enemy burst at ({}, {})", m_EnemyBurstSpawnPosition.x, m_EnemyBurstSpawnPosition.y);
}

int Stage1_1::GetRandomEnemyBurstInterval() const {
    const EnemyPlate& enemyPlate = Enemy::GetPlate();
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(enemyPlate.burstIntervalMin, enemyPlate.burstIntervalMax);
    return dis(gen);
}

void Stage1_1::CheckAndSpawnBoss() {
    // Boss 已生成過，不再重複檢查
    if (m_App->IsBossActive() || m_App->IsBossDefeated()) {
        return;
    }

    const bool scoreReached = m_App->GetCharacterScore() >= Boss1::kSpawnScoreThreshold;
    const bool timeReached = m_App->GetStageElapsedSeconds() >= Boss1::kSpawnTimeThreshold;

    if (!scoreReached && !timeReached) {
        return;
    }

    LOG_INFO("Boss1 spawn condition met (Score: {}, Elapsed: {}s)",
              m_App->GetCharacterScore(), m_App->GetStageElapsedSeconds());

    // 計算生成位置：螢幕右側中心
    const glm::vec2 cameraPos = m_App->GetCameraPos();
    const float viewportHalfWidth = m_App->GetCameraViewportHalfWidth();
    const glm::vec2 bossPos{cameraPos.x + viewportHalfWidth + 20.0f, 0.0f};

    m_App->SpawnBoss(std::make_shared<Boss1>(bossPos));
}