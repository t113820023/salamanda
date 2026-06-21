#include "stages/Stage1_2.hpp"
#include "App.hpp"
#include "Entity/Enemy.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include <random>

Stage1_2::Stage1_2(App* app)
    : m_App(app), m_IsStageComplete(false) {
}

void Stage1_2::Enter() {
    LOG_INFO("Stage 1-2: Entering");

    // 【強制先執行】重置關卡進行時間，防止沿用 Stage1-1 累積的時間
    // 必須在任何其他邏輯（包括 CheckAndSpawnBoss 的檢查）之前執行
    m_App->ResetStageElapsedSeconds();
    LOG_INFO("Stage 1-2: Stage elapsed time forcibly reset to 0.0f");

    // 重置 Boss 狀態，允許本關卡生成新的 Boss（Boss2）
    m_App->ResetBossState();

    // Switch background to 1-2 map
    m_App->SetBackgroundImage("/Image/1-2.png");

    // 設定本關卡使用的地圖橫幅
    m_App->SetBannerPaths("/Image/1-2地圖橫幅(上).png", "/Image/1-2地圖橫幅(下).png");

    // Spawn player character with starting health
    m_App->SpawnCharacter(2);

    // 顯示底部 HUD（生命值/分數/強化提示條）
    m_App->ShowHud();

    // Get enemy wave configuration from EnemyPlate
    const EnemyPlate& enemyPlate = Enemy::GetPlate();
    LOG_INFO("Stage 1-2: Enemy configuration - Wave delay: {}, Burst size: {}, Burst interval: {}-{}",
             enemyPlate.waveInitialDelay, enemyPlate.burstSize,
             enemyPlate.burstIntervalMin, enemyPlate.burstIntervalMax);

    // Reset stage completion flag
    m_IsStageComplete = false;
    m_IsWaitingForEnemyWave = true;
    m_EnemyWaveWaitCounter = 0;
    m_IsEnemyBurstActive = false;

    // 重置 Enemy2 生成計時
    m_Enemy2SpawnCounter = 0;
    m_Enemy2SpawnInterval = GetRandomEnemy2SpawnInterval();
}

void Stage1_2::Update(float deltaSeconds) {
    (void)deltaSeconds;

    // Boss 戰鬥已開始時，暫停一般敵人出怪（與 Stage1_1 相同）
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
                    m_App->SpawnEnemyAt(m_EnemyBurstSpawnPosition);
                    m_EnemyBurstCount++;
                    m_EnemyBurstSpawnCounter = 0;
                }
            }

            if (m_EnemyBurstCount >= enemyPlate.burstSize) {
                m_IsEnemyBurstActive = false;
                m_EnemyBurstInterval = GetRandomEnemyBurstInterval();
                m_EnemyBurstSpawnCounter = 0;
                LOG_INFO("Stage 1-2: Enemy burst complete. Next burst in {} frames", m_EnemyBurstInterval);
            }
        } else {
            if (!m_IsWaitingForEnemyWave) {
                m_EnemyBurstSpawnCounter++;
                if (m_EnemyBurstSpawnCounter >= m_EnemyBurstInterval) {
                    StartEnemyBurst();
                }
            }
        }

        // Enemy2 單一生成、隨機間隔（同樣在 Boss 戰開始後暫停）
        UpdateEnemy2Spawning();
    }

    // 由本關卡自行檢查 Boss2 生成條件（分數達門檻，或關卡持續時間達 5 分鐘）
    CheckAndSpawnBoss();

    // Boss2 被擊敗：進入結算階段（顯示分數、等待空白鍵返回封面），本關卡同時結束
    if (m_App->IsBossDefeated()) {
        LOG_INFO("Stage 1-2: Boss2 defeated! Entering ending screen.");
        m_App->EnterEndingScreen();
        m_IsStageComplete = true;
    }
}

void Stage1_2::Exit() {
    LOG_INFO("Stage 1-2: Exiting");
    m_App->HideHud();
    m_App->ClearBannerPaths();
}

bool Stage1_2::IsStageComplete() const {
    return m_IsStageComplete;
}

std::string Stage1_2::GetStageName() const {
    return "1-2";
}

void Stage1_2::StartEnemyBurst() {
    m_IsEnemyBurstActive = true;
    m_EnemyBurstCount = 0;
    m_EnemyBurstSpawnCounter = 0;
    m_EnemyBurstSpawnPosition = m_App->GetRandomEnemySpawnPosition();
    LOG_INFO("Stage 1-2: Starting new enemy burst at ({}, {})", m_EnemyBurstSpawnPosition.x, m_EnemyBurstSpawnPosition.y);
}

int Stage1_2::GetRandomEnemyBurstInterval() const {
    const EnemyPlate& enemyPlate = Enemy::GetPlate();
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(enemyPlate.burstIntervalMin, enemyPlate.burstIntervalMax);
    return dis(gen);
}

void Stage1_2::CheckAndSpawnBoss() {
    if (m_App->IsBossActive() || m_App->IsBossDefeated()) {
        return;
    }

    const bool scoreReached = m_App->GetCharacterScore() >= Boss2::kSpawnScoreThreshold;
    const bool timeReached = m_App->GetStageElapsedSeconds() >= Boss2::kSpawnTimeThreshold;

    if (!scoreReached && !timeReached) {
        return;
    }

    LOG_INFO("Stage 1-2: Boss2 spawn condition met (Score: {}, Elapsed: {}s)",
              m_App->GetCharacterScore(), m_App->GetStageElapsedSeconds());

    // Boss2 生成於畫面右側中心（Y 軸固定為 0，內部會自動調整使圖片右緣緊貼螢幕右邊界）
    const glm::vec2 cameraPos = m_App->GetCameraPos();
    const float viewportHalfWidth = m_App->GetCameraViewportHalfWidth();
    const glm::vec2 bossPos{cameraPos.x + viewportHalfWidth, 0.0f};

    m_App->SpawnBoss(std::make_shared<Boss2>(bossPos));
}

void Stage1_2::UpdateEnemy2Spawning() {
    if (m_Enemy2SpawnInterval <= 0) {
        m_Enemy2SpawnInterval = kEnemy2SpawnIntervalMin; // 防呆，避免初始化遺漏造成永遠不生成
    }

    m_Enemy2SpawnCounter++;
    if (m_Enemy2SpawnCounter >= m_Enemy2SpawnInterval) {
        const glm::vec2 spawnPos = m_App->GetRandomEnemySpawnPosition();
        m_App->SpawnEnemy2At(spawnPos);
        LOG_INFO("Stage 1-2: Enemy2 spawned at ({}, {})", spawnPos.x, spawnPos.y);

        m_Enemy2SpawnCounter = 0;
        m_Enemy2SpawnInterval = GetRandomEnemy2SpawnInterval();
    }
}

int Stage1_2::GetRandomEnemy2SpawnInterval() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(kEnemy2SpawnIntervalMin, kEnemy2SpawnIntervalMax);
    return dis(gen);
}