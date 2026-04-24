#include "App.hpp"

#include "Entity.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Core/Context.hpp"
#include <algorithm>
#include <random>

static bool IsColliding(const std::shared_ptr<Util::GameObject>& a,
                        const std::shared_ptr<Util::GameObject>& b) {
    glm::vec2 sizeA = a->GetScaledSize() * 0.5f;
    glm::vec2 sizeB = b->GetScaledSize() * 0.5f;
    glm::vec2 delta = glm::abs(a->m_Transform.translation - b->m_Transform.translation);

    return delta.x < (sizeA.x + sizeB.x) &&
                    delta.y < (sizeA.y + sizeB.y);
}

static glm::vec2 GetRandomEnemySpawnPosition(const std::unique_ptr<UGO::Graphics::Camera>& camera) {
    glm::vec2 cameraPos = camera ? camera->GetCameraPos() : glm::vec2{0.0f, 0.0f};
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> distX(150.0f, 260.0f);
    std::uniform_real_distribution<float> distY(-100.0f, 100.0f);
    return glm::vec2{cameraPos.x + distX(rng), distY(rng)};
}

static int GetRandomEnemyBurstInterval() {
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(App::kEnemyBurstIntervalMin, App::kEnemyBurstIntervalMax); // 2-4 秒
    return dist(rng);
}

void App::SpawnEnemyAt(const glm::vec2& position) {
    auto enemy = std::make_shared<Enemy>(position);
    m_Root.AddChild(enemy);
    m_Enemies.push_back(enemy);
    LOG_INFO("Enemy Spawned at ({}, {}), Total Enemies: {}", position.x, position.y, m_Enemies.size());
}

void App::StartEnemyBurst() {
    m_EnemyBurstSpawnPosition = GetRandomEnemySpawnPosition(m_Camera);
    m_LastEnemyPosition = m_EnemyBurstSpawnPosition;
    m_IsEnemyBurstActive = true;
    m_EnemyBurstCount = 0;
    m_EnemyBurstSpawnCounter = 0;
    m_EnemyBurstInterval = 0;
    LOG_INFO("Starting enemy burst at ({}, {})", m_EnemyBurstSpawnPosition.x, m_EnemyBurstSpawnPosition.y);
}

void App::Start() {
    LOG_TRACE("Start");
    Util::Logger::Init();
    m_CurrentState = State::UPDATE;

    // 載入背景圖片
    auto context = Core::Context::GetInstance();
    auto image = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/front.png");
    m_Background = std::make_shared<Util::GameObject>(image, 0.0f);  // z-index 0, 最底層

    // 設定縮放以填滿螢幕
    auto windowWidth = static_cast<float>(context->GetWindowWidth());
    auto windowHeight = static_cast<float>(context->GetWindowHeight());
    auto imageSize = image->GetSize();
    float scaleX = windowWidth / imageSize.x;
    float scaleY = windowHeight / imageSize.y;
    m_Background->m_Transform.scale = {scaleX, scaleY};

    m_Root.AddChild(m_Background);
}

void App::SpawnCharacter(int health) {
    m_Character = std::make_shared<Character>();
    m_Character->SetHealth(health);
    m_Character->SetScore(m_RespawnScore);
    m_Character->SetKills(m_RespawnKills);
    m_Character->m_Transform.translation = {0.0f, 0.0f};
    m_Root.AddChild(m_Character);

    m_IsWaitingForEnemyWave = true;
    m_EnemyWaveWaitCounter = 0;
}

void App::ClearSceneObjects() {
    for (auto& bullet : m_Bullets) {
        m_Root.RemoveChild(bullet);
    }
    m_Bullets.clear();

    for (auto& enemy : m_Enemies) {
        m_Root.RemoveChild(enemy);
    }
    m_Enemies.clear();

    for (auto& capsule : m_Capsules) {
        m_Root.RemoveChild(capsule);
    }
    m_Capsules.clear();

    if (m_Character) {
        m_Root.RemoveChild(m_Character);
        m_Character.reset();
    }
}

void App::SetBackgroundImage(const std::string& relativePath) {
    auto context = Core::Context::GetInstance();
    auto image = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + relativePath);
    m_Background->SetDrawable(image);

    const auto windowWidth = static_cast<float>(context->GetWindowWidth());
    const auto windowHeight = static_cast<float>(context->GetWindowHeight());
    const auto imageSize = image->GetSize();
    const float scaleX = windowWidth / imageSize.x;
    const float scaleY = windowHeight / imageSize.y;
    m_Background->m_Transform.scale = {scaleX, scaleY};
}

void App::CreateCapsule(const glm::vec2& position) {
    auto image = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/capsule.png");
    auto capsule = std::make_shared<Util::GameObject>(image, 0.0f);
    capsule->m_Transform.translation = position;
    capsule->m_Transform.scale = {3.0f, 3.0f};
    m_Root.AddChild(capsule);
    m_Capsules.push_back(capsule);
    LOG_INFO("Capsule spawned at ({}, {})", position.x, position.y);
}

void App::Update() {
    // 按下 Enter 後更新背景並建立角色
    if (Util::Input::IsKeyUp(Util::Keycode::RETURN) && !m_Character && !m_IsGameOver) {
        auto context = Core::Context::GetInstance();

        // 更換背景圖片
        auto newBackground = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/1-1.png");
        m_Background->SetDrawable(newBackground);

        // 重新計算背景縮放以填滿螢幕
        const auto windowWidth = static_cast<float>(context->GetWindowWidth());
        const auto windowHeight =
            static_cast<float>(context->GetWindowHeight());
        const auto imageSize = newBackground->GetSize();
        const float scaleX = windowWidth / imageSize.x;
        const float scaleY = windowHeight / imageSize.y;
        m_Background->m_Transform.scale = {scaleX, scaleY};

        // 在畫面中間新增角色
        SpawnCharacter(2);
        m_LastEnemyPosition = m_EnemyBurstSpawnPosition;
        LOG_INFO("Game started, character spawned and waiting to begin enemy wave");
    }

    /*
     * Do not touch the code below as they serve the purpose for
     * closing the window.
     */

    if (m_Character) {
        m_Character->Update();

        // 限制角色不超出相機視野
        if (m_Camera) {
            glm::vec2 cameraPos = m_Camera->GetCameraPos();
            float characterWidth = m_Character->GetScaledSize().x;
            float characterHeight = m_Character->GetScaledSize().y;
            
            LOG_INFO("Character Position: ({}, {}), Size: ({}, {}), Bullets: {}", 
                m_Character->m_Transform.translation.x, 
                m_Character->m_Transform.translation.y,
                characterWidth, characterHeight, m_Bullets.size());
            
            // 投影範圍：±128，所以視野寬度是256
            float leftBound = cameraPos.x - 128.0f + characterWidth * 0.5f;
            float rightBound = cameraPos.x + 128.0f - characterWidth * 0.5f;
            float topBound = 128.0f - characterHeight * 0.5f;
            float bottomBound = -128.0f + characterHeight * 0.5f;
            
            m_Character->m_Transform.translation.x = std::clamp(m_Character->m_Transform.translation.x, leftBound, rightBound);
            m_Character->m_Transform.translation.y = std::clamp(m_Character->m_Transform.translation.y, bottomBound, topBound);
        }

        // 獲取新生成的Bullets並添加到場景
        auto newBullets = m_Character->GetNewBullets();
        for (auto& bullet : newBullets) {
            m_Root.AddChild(bullet);
            m_Bullets.push_back(bullet);
            LOG_INFO("New Bullet Created! Total Bullets: {}", m_Bullets.size());
        }

        // 測試P鍵輸入
        static bool pPressed = false;
        if (Util::Input::IsKeyPressed(Util::Keycode::P)) {
            if (!pPressed) {
                LOG_INFO("P key is being pressed!");
                pPressed = true;
            }
        } else {
            pPressed = false;
        }
    }

    // 更新所有Bullets
    for (auto& bullet : m_Bullets) {
        bullet->Update();
    }

    // 更新所有Enemies
    for (auto& enemy : m_Enemies) {
        enemy->Update();
    }

    // 碰撞檢測：Bullet vs Enemy
    auto bulletIt = m_Bullets.begin();
    while (bulletIt != m_Bullets.end()) {
        bool bulletDestroyed = false;
        
        auto enemyIt = m_Enemies.begin();
        while (enemyIt != m_Enemies.end()) {
            if (IsColliding(*bulletIt, *enemyIt)) {
                // 子彈造成傷害
                (*enemyIt)->TakeDamage((*bulletIt)->GetDamage());
                
                if ((*enemyIt)->IsDead()) {
                    // 記錄最近消失的位置，並從場景中移除敵人
                    m_LastEnemyPosition = (*enemyIt)->m_Transform.translation;
                    m_Root.RemoveChild(*enemyIt);
                    enemyIt = m_Enemies.erase(enemyIt);
                    
                    // 角色獲得分數和擊殺數
                    if (m_Character) {
                        m_Character->AddScore(500);
                        m_Character->AddKill();
                        LOG_INFO("Enemy Killed! Score: {}, Kills: {}", m_Character->GetScore(), m_Character->GetKills());

                        if (m_Character->GetKills() % 5 == 0) {
                            m_Character->AddCapsuleCount(1);
                            m_Character->SetKills(0);  // 生成膠囊時擊殺紀錄器歸0
                            CreateCapsule(m_LastEnemyPosition);
                            LOG_INFO("Capsule earned! Total capsules: {}", m_Character->GetCapsuleCount());
                        }
                    }
                } else {
                    ++enemyIt;
                }
                
                // 子彈消失
                m_Root.RemoveChild(*bulletIt);
                bulletIt = m_Bullets.erase(bulletIt);
                bulletDestroyed = true;
                break;
            } else {
                ++enemyIt;
            }
        }
        
        if (!bulletDestroyed) {
            ++bulletIt;
        }
    }

    // 碰撞檢測：Character vs Enemy
    if (m_Character) {
        auto enemyIt = m_Enemies.begin();
        while (enemyIt != m_Enemies.end()) {
            if (IsColliding(m_Character, *enemyIt)) {
                // 角色受到傷害
                m_Character->DecreaseHealth();
                LOG_INFO("Character Hit! Health: {}", m_Character->GetHealth());
                
                // 敵人消失
                m_Root.RemoveChild(*enemyIt);
                enemyIt = m_Enemies.erase(enemyIt);
                
                if (m_Character->IsDead()) {
                    LOG_INFO("Character lost a life, respawning... Remaining Health: {}", m_Character->GetHealth());

                    if (m_Character->GetHealth() <= 0) {
                        LOG_INFO("Game Over! Final Score: {}, Kills: {}", 
                            m_Character->GetScore(), m_Character->GetKills());

                        m_IsGameOver = true;
                        m_IsCharacterRespawning = false;
                        m_CharacterRespawnCounter = 0;

                        ClearSceneObjects();
                        SetBackgroundImage("/Image/gameover.png");
                        return;
                    }

                    m_RespawnHealth = m_Character->GetHealth();
                    m_RespawnScore = m_Character->GetScore();
                    m_RespawnKills = m_Character->GetKills();
                    m_Root.RemoveChild(m_Character);
                    m_Character.reset();
                    m_IsCharacterRespawning = true;
                    m_CharacterRespawnCounter = 0;
                }
            } else {
                ++enemyIt;
            }
        }
    }

    // 碰撞檢測：Character vs Capsule
    if (m_Character) {
        auto capsuleIt = m_Capsules.begin();
        while (capsuleIt != m_Capsules.end()) {
            if (IsColliding(m_Character, *capsuleIt)) {
                // 膠囊消失
                m_Root.RemoveChild(*capsuleIt);
                capsuleIt = m_Capsules.erase(capsuleIt);
                LOG_INFO("Capsule collected! Remaining capsules: {}", m_Capsules.size());
            } else {
                ++capsuleIt;
            }
        }
    }

    // 移除超出螢幕的Bullets
    glm::vec2 cameraPos = m_Camera ? m_Camera->GetCameraPos() : glm::vec2{0.0f, 0.0f};
    
    // 顯示相機中心在地圖上的座標（以背景中心為0）
    int capsuleCount = m_Character ? m_Character->GetCapsuleCount() : 0;
    LOG_INFO("Camera Center Position: ({}, {}), Capsules: {}", cameraPos.x, cameraPos.y, capsuleCount);
    
    // 移除超出相機視野或飛行時間已結束的子彈並從場景中移除
    if (m_Camera) {
        auto it = m_Bullets.begin();
        while (it != m_Bullets.end()) {
            if ((*it)->IsOffScreen(m_Camera->GetCameraPos()) || (*it)->IsOutOfLife()) {
                m_Root.RemoveChild(*it);  // 從場景樹中移除
                it = m_Bullets.erase(it);
                LOG_INFO("Bullet Removed! Total Bullets: {}", m_Bullets.size());
            } else {
                ++it;
            }
        }
    }

    // 移除超出螢幕的Enemies
    auto enemyIt = m_Enemies.begin();
    while (enemyIt != m_Enemies.end()) {
        if ((*enemyIt)->IsOffScreen(cameraPos)) {
            LOG_INFO("Enemy off screen at ({}, {}), removing...", 
                (*enemyIt)->m_Transform.translation.x, (*enemyIt)->m_Transform.translation.y);
            m_Root.RemoveChild(*enemyIt);
            enemyIt = m_Enemies.erase(enemyIt);
            
            LOG_INFO("Enemy Removed! Total Enemies: {}", m_Enemies.size());
        } else {
            ++enemyIt;
        }
    }

    // 角色重新生成邏輯
    if (m_IsCharacterRespawning && !m_IsGameOver) {
        m_CharacterRespawnCounter++;
        if (m_CharacterRespawnCounter >= 30) {
            SpawnCharacter(m_RespawnHealth);
            m_IsCharacterRespawning = false;
            m_CharacterRespawnCounter = 0;
            LOG_INFO("Character Respawned with Health: {}", m_RespawnHealth);
        }
    }

    // 敵人生成波次邏輯
    if (!m_IsGameOver && m_Character) {
        if (m_IsWaitingForEnemyWave) {
            m_EnemyWaveWaitCounter++;
            if (m_EnemyWaveWaitCounter >= App::kEnemyWaveInitialDelay) {
                m_IsWaitingForEnemyWave = false;
                StartEnemyBurst();
            }
        }

        if (m_IsEnemyBurstActive) {
            if (m_EnemyBurstCount < App::kEnemyBurstSize) {
                m_EnemyBurstSpawnCounter++;
                if (m_EnemyBurstSpawnCounter >= App::kEnemyBurstSpawnDelay) {
                    SpawnEnemyAt(m_EnemyBurstSpawnPosition);
                    m_EnemyBurstCount++;
                    m_EnemyBurstSpawnCounter = 0;
                }
            }

            if (m_EnemyBurstCount >= App::kEnemyBurstSize) {
                m_IsEnemyBurstActive = false;
                m_EnemyBurstInterval = GetRandomEnemyBurstInterval();
                m_EnemyBurstSpawnCounter = 0;
                LOG_INFO("Enemy burst complete. Next burst in {} frames", m_EnemyBurstInterval);
            }
        } else {
            if (!m_IsWaitingForEnemyWave) {
                m_EnemyBurstSpawnCounter++;
                if (m_EnemyBurstSpawnCounter >= m_EnemyBurstInterval) {
                    StartEnemyBurst();
                }
            }
        }
    }

    m_Root.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}
