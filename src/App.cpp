#include "App.hpp"
#include "Entity.hpp"
#include "Entity/Missile.hpp"
#include "Entity/Enemy2.hpp"
#include "Entity/Fireball.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "Core/Context.hpp"
#include "StageManager.hpp"
#include "MapObject.hpp"
#include <algorithm>
#include <limits>
#include <random>

static bool IsColliding(const std::shared_ptr<Util::GameObject>& a,
                        const std::shared_ptr<Util::GameObject>& b) {
    glm::vec2 sizeA = a->GetScaledSize() * 0.5f;
    glm::vec2 sizeB = b->GetScaledSize() * 0.5f;
    glm::vec2 delta = glm::abs(a->m_Transform.translation - b->m_Transform.translation);
    return delta.x < (sizeA.x + sizeB.x) && delta.y < (sizeA.y + sizeB.y);
}

void App::Start() {
    LOG_TRACE("Start");
    m_CurrentState = State::UPDATE;
    
    std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    m_Camera = std::make_unique<UGO::Graphics::Camera>(
        static_cast<float>(context->GetWindowWidth()),
        static_cast<float>(context->GetWindowHeight()),
        100.0f
    );
    
    auto image = std::make_shared<Util::Image>(RESOURCE_DIR "/Image/front.png");
    m_Background = std::make_shared<Util::GameObject>(image, 0.0f);

    // 設定縮放以填滿螢幕
    auto windowWidth = static_cast<float>(context->GetWindowWidth());
    auto windowHeight = static_cast<float>(context->GetWindowHeight());
    auto imageSize = image->GetSize();
    float scaleX = windowWidth / imageSize.x;
    float scaleY = windowHeight / imageSize.y;
    m_Background->m_Transform.scale = {scaleX, scaleY};
    m_Background->m_Transform.translation = {0.0f, 0.0f}; 
    m_Root.AddChild(m_Background);

    m_StageManager = std::make_unique<StageManager>(this);

    InitHud();
}

void App::Update() {
    float deltaSeconds = Util::Time::GetDeltaTimeMs() / 1000.0f;

    // Game Over 階段：完全跳過遊戲邏輯，只處理 Game Over 專屬輸入（空白鍵切換背景、F 鍵重啟/返回）
    if (m_IsGameOver) {
        // 進入 Game Over 時，死亡瞬間可能還按著 F 或 SPACE（例如正在升級/射擊）。
        // 必須先偵測到這兩個鍵都放開過一次，才開始接受 Game Over 畫面的輸入，
        // 避免殘留的按鍵在放開瞬間被誤判為「在 Game Over 畫面按下該鍵」。
        if (!m_GameOverInputReady) {
            if (!Util::Input::IsKeyPressed(Util::Keycode::F) &&
                !Util::Input::IsKeyPressed(Util::Keycode::SPACE)) {
                m_GameOverInputReady = true;
            }
        } else {
            HandleGameOverInput();
        }

        // 即使跳過遊戲邏輯，仍須呼叫 Update() 讓場景樹（背景圖片）持續被渲染，
        // 否則畫面會停留在進入 Game Over 那一幀之後就不再更新，呈現黑畫面。
        m_Root.Update();

        if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
            m_CurrentState = State::END;
        }
        return;
    }

    // 結算階段：與 Game Over 同等級的全域階段，完全跳過所有關卡邏輯，
    // 只處理空白鍵返回封面。同樣需要按鍵狀態偵測，避免擊敗 Boss2 瞬間
    // 殘留的 SPACE（例如剛好正在射擊）被誤判為結算畫面的操作。
    if (m_IsEnding) {
        if (!m_EndingInputReady) {
            if (!Util::Input::IsKeyPressed(Util::Keycode::SPACE)) {
                m_EndingInputReady = true;
            }
        } else {
            HandleEndingInput();
        }

        m_Root.Update();

        if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
            m_CurrentState = State::END;
        }
        return;
    }

    // 按下 Enter 後啟動 Stage 1-1
    if (Util::Input::IsKeyUp(Util::Keycode::RETURN) && !m_StageManager->HasActiveStage()) {
        LOG_INFO("Starting Stage 1-1");
        m_StageElapsedSeconds = 0.0f;
        m_StageManager->StartStage("1-1");
    }

    if (m_Camera) {
        m_Camera->Update(deltaSeconds);
    }

    // 更新當前關卡邏輯 (由 Stage1_1 去計算並管理出怪)
    if (m_StageManager) {
         m_StageManager->Update(deltaSeconds);
    }

    if (m_Character) {
        m_Character->Update();

        // 限制角色不超出相機視野
        if (m_Camera) {
            glm::vec2 cameraPos = m_Camera->GetCameraPos();
            float characterWidth = m_Character->GetScaledSize().x;
            float characterHeight = m_Character->GetScaledSize().y;
            float viewportHalfWidth = m_Camera->GetViewportHalfWidth();
            float viewportHalfHeight = m_Camera->GetViewportHalfHeight();
            
            LOG_INFO("Character Position: ({}, {}), Size: ({}, {}), Bullets: {}", 
                m_Character->m_Transform.translation.x, 
                m_Character->m_Transform.translation.y,
                characterWidth, characterHeight, m_Bullets.size());
            
            float leftBound = cameraPos.x - viewportHalfWidth + characterWidth * 0.5f;
            float rightBound = cameraPos.x + viewportHalfWidth - characterWidth * 0.5f;
            float topBound = viewportHalfHeight - characterHeight * 0.5f;
            float bottomBound = -viewportHalfHeight + characterHeight * 0.5f;
            
            m_Character->m_Transform.translation.x = std::clamp(m_Character->m_Transform.translation.x, leftBound, rightBound);
            m_Character->m_Transform.translation.y = std::clamp(m_Character->m_Transform.translation.y, bottomBound, topBound);
        }

        // 獲取新生成的 Bullets 並添加到場景
        auto newBullets = m_Character->GetNewBullets();
        for (auto& bullet : newBullets) {
            m_Root.AddChild(bullet);
            m_Bullets.push_back(bullet);
            LOG_INFO("New Bullet Created! Total Bullets: {}", m_Bullets.size());
        }

        // 獲取新生成的飛彈並添加到場景
        auto newMissiles = m_Character->GetNewMissiles();
        for (auto& missile : newMissiles) {
            m_Root.AddChild(missile);
            m_Missiles.push_back(missile);
            LOG_INFO("New Missile Created! Total Missiles: {}", m_Missiles.size());
        }

        // 測試 O 鍵輸入：給予 999 次防碰撞次數，同時啟用子彈傷害提升（永久生效，直到重生/重啟）
        static bool oPressed = false;
        if (Util::Input::IsKeyPressed(Util::Keycode::O)) {
            if (!oPressed) {
                if (m_Character) {
                    m_Character->AddCollisionImmunity(999);
                    m_Character->ActivateBulletDamageBoost();
                    LOG_INFO("O key pressed! Added 999 collision immunity. Total: {}", m_Character->GetCollisionImmunityCount());
                    LOG_INFO("O key pressed! Bullet damage boost activated (all bullets now deal {} damage).", Character::kBoostedBulletDamage);
                }
                oPressed = true;
            }
        } else {
            oPressed = false;
        }

        // 測試 P 鍵輸入：強制進入 BOSS 戰
        static bool pPressed = false;
        if (Util::Input::IsKeyPressed(Util::Keycode::P)) {
            if (!pPressed) {
                LOG_INFO("P key pressed! Forcing BOSS spawn...");
                if (!m_BossSpawned && m_StageManager) {
                    std::string currentStageName = m_StageManager->GetCurrentStageName();
                    if (currentStageName == "1-1") {
                        auto boss1 = std::make_shared<Boss1>(glm::vec2{m_Camera->GetCameraPos().x + m_Camera->GetViewportHalfWidth() + 20.0f, 0.0f});
                        SpawnBoss(boss1);
                        LOG_INFO("Boss1 forcibly spawned!");
                    } else if (currentStageName == "1-2") {
                        auto boss2 = std::make_shared<Boss2>(glm::vec2{m_Camera->GetCameraPos().x + m_Camera->GetViewportHalfWidth(), 0.0f});
                        SpawnBoss(boss2);
                        LOG_INFO("Boss2 forcibly spawned!");
                    } else {
                        LOG_WARN("Unknown stage: {}, cannot spawn boss", currentStageName);
                    }
                } else {
                    LOG_WARN("Boss already spawned or stage manager not available");
                }
                pPressed = true;
            }
        } else {
            pPressed = false;
        }

        if (m_IsCharacterCollisionDisabled) {
            m_CharacterCollisionDisableCounter++;
            if (m_CharacterCollisionDisableCounter >= App::kCharacterCollisionDisableDuration) {
                m_IsCharacterCollisionDisabled = false;
                m_CharacterCollisionDisableCounter = 0;
                LOG_INFO("Character collision re-enabled");
            }
        }
    }

    // 更新所有 Bullets
    for (auto& bullet : m_Bullets) {
        bullet->Update();
    }

    // 更新所有飛彈
    for (auto& missile : m_Missiles) {
        missile->Update();
    }

    // 更新所有火球（Boss2 發射的攻擊物）
    for (auto& fireball : m_Fireballs) {
        fireball->Update();
    }

    // 更新所有 Enemies（傳入角色目前位置，供需要追蹤/直線衝刺的敵人型別使用）
    const glm::vec2 characterPosForEnemies = m_Character ? m_Character->m_Transform.translation : m_LastEnemyPosition;
    for (auto& enemy : m_Enemies) {
        enemy->Update(characterPosForEnemies);
    }

    // 更新膠囊（與地圖同步向左移動）
    for (auto& capsule : m_Capsules) {
        capsule->m_Transform.translation.x -= App::kMapMoveSpeed * deltaSeconds;
    }

    // 更新地圖橫幅（隨地圖向左移動，並持續鋪滿視野右側）
    // 是否啟用橫幅完全由目前關卡透過 SetBannerPaths() 決定，不寫死關卡背景路徑。
    if (m_Camera && !m_TopBannerPath.empty() && !m_BottomBannerPath.empty()) {
        const auto cameraPos = m_Camera->GetCameraPos();
        const float viewportHalfWidth = m_Camera->GetViewportHalfWidth();
        const float rightBound = cameraPos.x + viewportHalfWidth;

        for (auto& mapObj : m_MapObjects) {
            mapObj->Update(deltaSeconds, App::kMapMoveSpeed);
        }

        auto ensureBannerStream = [&](MapObject::Type type, const std::string& relativePath) {
            float rightmostRightEdge = std::numeric_limits<float>::lowest();
            bool found = false;
            for (auto& mo : m_MapObjects) {
                if (mo->GetType() == type) {
                    found = true;
                    float rightEdge = mo->m_Transform.translation.x + mo->GetHalfWidth();
                    if (rightEdge > rightmostRightEdge) {
                        rightmostRightEdge = rightEdge;
                    }
                }
            }

            if (!found) {
                auto tile = CreateMapBannerAtLeft(relativePath, type == MapObject::Type::BannerTop, rightBound);
                rightmostRightEdge = tile->m_Transform.translation.x + tile->GetHalfWidth();
            }

            while (rightmostRightEdge < rightBound + 0.001f) {
                auto tile = CreateMapBannerAtLeft(relativePath, type == MapObject::Type::BannerTop, rightmostRightEdge);
                rightmostRightEdge = tile->m_Transform.translation.x + tile->GetHalfWidth();
            }
        };

        ensureBannerStream(MapObject::Type::BannerTop, m_TopBannerPath);
        ensureBannerStream(MapObject::Type::BannerBottom, m_BottomBannerPath);
    }

    // 累計關卡持續時間（僅在角色存在、Boss 尚未生成時計時；供 Stage 自行判斷生成條件使用）
    if (m_Character && !m_BossSpawned) {
        m_StageElapsedSeconds += deltaSeconds;
    }

    // 更新 Boss（若已生成）
    if (m_Boss) {
        m_Boss->Update(m_Character);

        // 泛用攻擊通知機制：若 Boss 表示該生成附屬攻擊物，在其目前位置生成火球
        if (m_Boss->HasPendingAttack()) {
            auto fireball = std::make_shared<Fireball>(m_Boss->m_Transform.translation);
            m_Fireballs.push_back(fireball);
            m_Root.AddChild(fireball);
            m_Boss->ClearPendingAttack();
        }
    }

    // 碰撞檢測：Bullet vs Enemy
    auto bulletIt = m_Bullets.begin();
    while (bulletIt != m_Bullets.end()) {
        bool bulletDestroyed = false;
        
        auto enemyIt = m_Enemies.begin();
        while (enemyIt != m_Enemies.end()) {
            if (IsColliding(*bulletIt, *enemyIt)) {
                (*enemyIt)->TakeDamage((*bulletIt)->GetDamage());
                
                if ((*enemyIt)->IsDead()) {
                    m_LastEnemyPosition = (*enemyIt)->m_Transform.translation;
                    m_Root.RemoveChild(*enemyIt);
                    enemyIt = m_Enemies.erase(enemyIt);
                    
                    if (m_Character) {
                        m_Character->AddScore(500);
                        m_Character->AddKill();
                        LOG_INFO("Enemy Killed! Score: {}, Kills: {}", m_Character->GetScore(), m_Character->GetKills());

                        if (m_Character->GetKills() % 5 == 0) {
                            m_Character->SetKills(0); 
                            CreateCapsule(m_LastEnemyPosition);
                            LOG_INFO("Capsule spawned at enemy position");
                        }
                    }
                } else {
                    ++enemyIt;
                }
                
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
    if (m_Character && !m_IsCharacterCollisionDisabled) {
        auto enemyIt = m_Enemies.begin();
        while (enemyIt != m_Enemies.end()) {
            if (IsColliding(m_Character, *enemyIt)) {
                m_Root.RemoveChild(*enemyIt);
                enemyIt = m_Enemies.erase(enemyIt);

                HandleCharacterHit();
                if (!m_Character) {
                    break; // 角色已死亡或正在重生，停止本輪碰撞檢測
                }
            } else {
                ++enemyIt;
            }
        }
    }

    // 碰撞檢測：Bullet vs Boss
    // 子彈撞到 Boss 一律消失（視覺上仍有命中效果），但只有 Boss 自己回報
    // CanBeDamaged() 為 true 時才真正造成傷害（例如 Boss2 需要場上還有自己
    // 發出的火球才允許受傷；Boss1 等沒有此限制的 Boss 永遠回傳 true）。
    if (m_Boss) {
        const bool canDamageBoss = m_Boss->CanBeDamaged();

        auto bulletIt = m_Bullets.begin();
        while (bulletIt != m_Bullets.end()) {
            if (IsColliding(*bulletIt, m_Boss)) {
                if (canDamageBoss) {
                    m_Boss->TakeDamage((*bulletIt)->GetDamage());
                    LOG_INFO("Boss Hit! Health: {}", m_Boss->GetHealth());
                } else {
                    LOG_INFO("Bullet hit Boss while invulnerable, no damage dealt.");
                }

                m_Root.RemoveChild(*bulletIt);
                bulletIt = m_Bullets.erase(bulletIt);

                if (canDamageBoss && m_Boss->IsDead()) {
                    if (m_Character) {
                        m_Character->AddScore(5000); 
                        LOG_INFO("Boss Defeated! Score: {}", m_Character->GetScore());
                    }
                    m_Root.RemoveChild(m_Boss);
                    m_Boss.reset();
                    m_BossSpawned = false;
                    m_BossDefeated = true;
                    break; // m_Boss 已失效（nullptr），必須立即停止本迴圈，
                           // 否則下一次迭代呼叫 IsColliding(*bulletIt, m_Boss) 會對空指標解參考而崩潰。
                }
            } else {
                ++bulletIt;
            }
        }
    }

    // 碰撞檢測：Character vs Boss
    if (m_Character && m_Boss && !m_IsCharacterCollisionDisabled) {
        if (IsColliding(m_Character, m_Boss)) {
            LOG_INFO("Character Hit by Boss!");
            HandleCharacterHit();
        }
    }

    // 碰撞檢測：Fireball vs Character（火球只與角色互動，不與玩家子彈/飛彈碰撞）
    if (m_Character && !m_IsCharacterCollisionDisabled) {
        auto fireballIt = m_Fireballs.begin();
        while (fireballIt != m_Fireballs.end()) {
            if (IsColliding(m_Character, *fireballIt)) {
                LOG_INFO("Character hit by Fireball!");
                m_Root.RemoveChild(*fireballIt);
                fireballIt = m_Fireballs.erase(fireballIt);

                HandleCharacterHit();
                if (!m_Character) {
                    break; // 角色已死亡或正在重生，停止本輪碰撞檢測
                }
            } else {
                ++fireballIt;
            }
        }
    }

    // 碰撞檢測：飛彈 vs Enemy
    auto missileIt = m_Missiles.begin();
    while (missileIt != m_Missiles.end()) {
        bool missileDestroyed = false;
        
        auto enemyIt = m_Enemies.begin();
        while (enemyIt != m_Enemies.end()) {
            if (IsColliding(*missileIt, *enemyIt)) {
                (*enemyIt)->TakeDamage((*missileIt)->GetDamage());
                
                if ((*enemyIt)->IsDead()) {
                    m_LastEnemyPosition = (*enemyIt)->m_Transform.translation;
                    m_Root.RemoveChild(*enemyIt);
                    enemyIt = m_Enemies.erase(enemyIt);
                    
                    if (m_Character) {
                        m_Character->AddScore(500);
                        m_Character->AddKill();
                        LOG_INFO("Enemy Killed by Missile! Score: {}, Kills: {}", m_Character->GetScore(), m_Character->GetKills());

                        if (m_Character->GetKills() % 5 == 0) {
                            m_Character->SetKills(0); 
                            CreateCapsule(m_LastEnemyPosition);
                            LOG_INFO("Capsule spawned at enemy position");
                        }
                    }
                } else {
                    ++enemyIt;
                }
                
                m_Root.RemoveChild(*missileIt);
                missileIt = m_Missiles.erase(missileIt);
                missileDestroyed = true;
                break;
            } else {
                ++enemyIt;
            }
        }
        
        if (!missileDestroyed) {
            ++missileIt;
        }
    }

    // 碰撞檢測：飛彈 vs Boss
    if (m_Boss) {
        const bool canDamageBoss = m_Boss->CanBeDamaged();

        auto missileIt = m_Missiles.begin();
        while (missileIt != m_Missiles.end()) {
            if (IsColliding(*missileIt, m_Boss)) {
                if (canDamageBoss) {
                    m_Boss->TakeDamage((*missileIt)->GetDamage());
                    LOG_INFO("Boss Hit by Missile! Health: {}", m_Boss->GetHealth());
                } else {
                    LOG_INFO("Missile hit Boss while invulnerable, no damage dealt.");
                }

                m_Root.RemoveChild(*missileIt);
                missileIt = m_Missiles.erase(missileIt);

                if (canDamageBoss && m_Boss->IsDead()) {
                    if (m_Character) {
                        m_Character->AddScore(5000); 
                        LOG_INFO("Boss Defeated! Score: {}", m_Character->GetScore());
                    }
                    m_Root.RemoveChild(m_Boss);
                    m_Boss.reset();
                    m_BossSpawned = false;
                    m_BossDefeated = true;
                    break; // 同上：m_Boss 已失效，必須立即停止本迴圈避免空指標解參考。
                }
            } else {
                ++missileIt;
            }
        }
    }

    // 碰撞檢測：Character vs Capsule
    if (m_Character) {
        auto capsuleIt = m_Capsules.begin();
        while (capsuleIt != m_Capsules.end()) {
            if (IsColliding(m_Character, *capsuleIt)) {
                m_Root.RemoveChild(*capsuleIt);
                capsuleIt = m_Capsules.erase(capsuleIt);
                m_Character->AddCapsuleCount(1);
                LOG_INFO("Capsule collected! Total capsules: {}, Remaining capsules: {}",
                    m_Character->GetCapsuleCount(), m_Capsules.size());
            } else {
                ++capsuleIt;
            }
        }
    }

    // 移除超出螢幕的 Bullets 與飛彈
    glm::vec2 cameraPos = m_Camera ? m_Camera->GetCameraPos() : glm::vec2{0.0f, 0.0f};
       
    if (m_Camera) {
        auto it = m_Bullets.begin();
        while (it != m_Bullets.end()) {
            if ((*it)->IsOffScreen(m_Camera->GetCameraPos(), m_Camera->GetViewportHalfWidth(), m_Camera->GetViewportHalfHeight()) || (*it)->IsOutOfLife()) {
                m_Root.RemoveChild(*it);  
                it = m_Bullets.erase(it);
                LOG_INFO("Bullet Removed! Total Bullets: {}", m_Bullets.size());
            } else {
                ++it;
            }
        }

        auto mslIt = m_Missiles.begin();
        while (mslIt != m_Missiles.end()) {
            if ((*mslIt)->IsOffScreen(m_Camera->GetCameraPos(), m_Camera->GetViewportHalfWidth(), m_Camera->GetViewportHalfHeight()) || (*mslIt)->IsOutOfLife()) {
                m_Root.RemoveChild(*mslIt);  
                mslIt = m_Missiles.erase(mslIt);
                LOG_INFO("Missile Removed! Total Missiles: {}", m_Missiles.size());
            } else {
                ++mslIt;
            }
        }

        // 移除飛出畫面的火球
        auto fireballIt = m_Fireballs.begin();
        while (fireballIt != m_Fireballs.end()) {
            if ((*fireballIt)->IsOffScreen(m_Camera->GetCameraPos(), m_Camera->GetViewportHalfWidth(), m_Camera->GetViewportHalfHeight())) {
                m_Root.RemoveChild(*fireballIt);
                fireballIt = m_Fireballs.erase(fireballIt);
                LOG_INFO("Fireball Removed! Total Fireballs: {}", m_Fireballs.size());
            } else {
                ++fireballIt;
            }
        }
    }

    // 將這一幀最終的火球數量回報給 Boss（泛用機制，與角色碰撞、飛出畫面消失都已處理完畢）
    if (m_Boss) {
        m_Boss->NotifyAttackObjectCount(static_cast<int>(m_Fireballs.size()));
    }

    // 移除超出螢幕的 Enemies
    auto enemyIt = m_Enemies.begin();
    while (enemyIt != m_Enemies.end()) {
        if ((*enemyIt)->IsOffScreen(cameraPos, m_Camera ? m_Camera->GetViewportHalfWidth() : 640.0f, m_Camera ? m_Camera->GetViewportHalfHeight() : 360.0f)) {
            LOG_INFO("Enemy off screen at ({}, {}), removing...", 
                (*enemyIt)->m_Transform.translation.x, (*enemyIt)->m_Transform.translation.y);
            m_Root.RemoveChild(*enemyIt);
            enemyIt = m_Enemies.erase(enemyIt);
            LOG_INFO("Enemy Removed! Total Enemies: {}", m_Enemies.size());
        } else {
            ++enemyIt;
        }
    }

    // 移除超出螢幕的 Capsules
    if (m_Camera) {
        auto capsuleIt = m_Capsules.begin();
        const float leftBound = cameraPos.x - m_Camera->GetViewportHalfWidth();
        float viewportHalfHeight = m_Camera->GetViewportHalfHeight();
        while (capsuleIt != m_Capsules.end()) {
            const auto& capsule = *capsuleIt;
            const auto& pos = capsule->m_Transform.translation;
            if (pos.x < leftBound || pos.y < -viewportHalfHeight || pos.y > viewportHalfHeight) {
                m_Root.RemoveChild(capsule);
                capsuleIt = m_Capsules.erase(capsuleIt);
                LOG_INFO("Capsule Removed! Total Capsules: {}", m_Capsules.size());
            } else {
                ++capsuleIt;
            }
        }

        // 移除超出螢幕的地圖切片（包含橫幅）
        auto mapIt = m_MapObjects.begin();
        while (mapIt != m_MapObjects.end()) {
            const auto& mapObj = *mapIt;
            const auto& pos = mapObj->m_Transform.translation;
            const float halfW = mapObj->GetHalfWidth();
            if (pos.x + halfW < leftBound - 0.001f) {
                m_Root.RemoveChild(mapObj);
                mapIt = m_MapObjects.erase(mapIt);
                LOG_INFO("Map tile Removed! Total Map Tiles: {}", m_MapObjects.size());
            } else {
                ++mapIt;
            }
        }
    }

    // 角色重新生成邏輯
    if (m_IsCharacterRespawning && !m_IsGameOver) {
        m_CharacterRespawnCounter++;
        if (m_CharacterRespawnCounter >= 30) {
            SpawnCharacter(m_RespawnHealth);
            m_IsCharacterRespawning = false;
            m_CharacterRespawnCounter = 0;
            m_IsCharacterCollisionDisabled = true;
            m_CharacterCollisionDisableCounter = 0;
            LOG_INFO("Character Respawned with Health: {} and temporary collision disabled", m_RespawnHealth);
        }
    }

    // 【修改】原本位於此處的舊隨機生成波次程式碼已完全移除，因為已被轉移至 Stage1_1.cpp。

    UpdateHud();

    m_Root.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}

// --- 提供給關卡的 API 與其他私有函式實作 ---

void App::SetBackgroundImage(const std::string& relativePath) {
    if (relativePath != m_CurrentBackgroundPath) {
        ClearMapBanners();
    }
    m_CurrentBackgroundPath = relativePath;

    auto context = Core::Context::GetInstance();
    auto newBackground = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + relativePath);
    if (m_Background) {
        m_Background->SetDrawable(newBackground);

        // 重新計算縮放，確保新背景同樣填滿視窗
        const auto windowWidth = static_cast<float>(context->GetWindowWidth());
        const auto windowHeight = static_cast<float>(context->GetWindowHeight());
        const auto imageSize = newBackground->GetSize();
        const float scaleX = windowWidth / imageSize.x;
        const float scaleY = windowHeight / imageSize.y;
        m_Background->m_Transform.scale = {scaleX, scaleY};
    }
}

void App::SetBannerPaths(const std::string& topBannerPath, const std::string& bottomBannerPath) {
    m_TopBannerPath = topBannerPath;
    m_BottomBannerPath = bottomBannerPath;
}

void App::ClearBannerPaths() {
    m_TopBannerPath.clear();
    m_BottomBannerPath.clear();
    ClearMapBanners();
}

void App::SpawnCharacter(int health) {
    if (!m_Character) {
        m_Character = std::make_shared<Character>();
        m_Character->SetHealth(health);
        m_Character->SetScore(m_RespawnScore);
        m_Character->SetKills(m_RespawnKills);
        m_Character->m_Transform.translation = {0.0f, 0.0f}; // 重生於畫面正中心
        m_Root.AddChild(m_Character);
    }
}

void App::SpawnEnemyAt(glm::vec2 position) {
    auto enemy = std::make_shared<Enemy>(position);
    m_Enemies.push_back(enemy);
    m_Root.AddChild(enemy);
}

void App::SpawnEnemy2At(glm::vec2 position) {
    auto enemy = std::make_shared<Enemy2>(position);
    m_Enemies.push_back(enemy);
    m_Root.AddChild(enemy);
}

namespace {
constexpr float kHudZIndex = 10.0f;
constexpr const char* kHudFontPath = RESOURCE_DIR "/Fonts/Inter.ttf";
constexpr int kHudFontSize = 20;
constexpr int kHudSmallFontSize = 16;

const std::array<std::string, App::kHudPowerCellCount> kHudPowerCellNames = {
    "SPEED", "MISSILE", "SOUND", "LASER", "IMMUNITY"
};
} // namespace

void App::InitHud() {
    if (m_HudInitialized) {
        return;
    }
    m_HudInitialized = true;

    // 左側：第一行 "1P  HP:2" 形式（生命值直接以文字顯示，跟著血量更新）
    m_HudLifeText = std::make_shared<Util::Text>(kHudFontPath, kHudFontSize, "1P  HP:2", Util::Color(255, 255, 255));
    m_HudLifeLabel = std::make_shared<Util::GameObject>(m_HudLifeText, kHudZIndex);
    m_HudLifeLabel->SetVisible(false);
    m_Root.AddChild(m_HudLifeLabel);

    // 左側：第二行 "1P" + 完整分數
    m_HudScoreText = std::make_shared<Util::Text>(kHudFontPath, kHudFontSize, "1P0000000", Util::Color(255, 255, 255));
    m_HudScoreLabel = std::make_shared<Util::GameObject>(m_HudScoreText, kHudZIndex);
    m_HudScoreLabel->SetVisible(false);
    m_Root.AddChild(m_HudScoreLabel);

    // 右側：5 個強化選項格（反白背景 + 文字）
    // 注意：先建立文字物件，取得其實際渲染尺寸後，再依此縮放背景，
    // 確保 99x21 的 UI_listback.png 背景能完整包住較長的字串（例如 IMMUNITY）。
    constexpr float kHudCellPaddingX = 12.0f; // 背景左右各留的內距
    constexpr float kHudCellPaddingY = 6.0f;  // 背景上下各留的內距

    for (int i = 0; i < kHudPowerCellCount; ++i) {
        auto labelDrawable = std::make_shared<Util::Text>(
            kHudFontPath, kHudSmallFontSize, kHudPowerCellNames[i], Util::Color(77, 166, 255));
        auto label = std::make_shared<Util::GameObject>(
            labelDrawable,
            kHudZIndex + 0.01f); // 文字疊在反白背景之上，z-index 稍大一點確保不被背景蓋住
        label->SetVisible(false);
        m_Root.AddChild(label);
        m_HudPowerCellLabels[i] = label;

        auto backgroundDrawable = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/UI_listback.png");
        auto background = std::make_shared<Util::GameObject>(backgroundDrawable, kHudZIndex);

        // 依文字實際寬高，動態縮放背景，使其至少能包住文字 + 內距
        const glm::vec2 textSize = labelDrawable->GetSize();
        const glm::vec2 bgOriginalSize = backgroundDrawable->GetSize();
        const float desiredWidth = textSize.x + kHudCellPaddingX * 2.0f;
        const float desiredHeight = textSize.y + kHudCellPaddingY * 2.0f;
        background->m_Transform.scale = {
            desiredWidth / bgOriginalSize.x,
            desiredHeight / bgOriginalSize.y
        };

        background->SetVisible(false);
        m_Root.AddChild(background);
        m_HudPowerCellBackgrounds[i] = background;
    }
}

glm::vec2 App::GetRandomEnemySpawnPosition() const {
    glm::vec2 cameraPos = m_Camera ? m_Camera->GetCameraPos() : glm::vec2{0.0f, 0.0f};
    float viewportHalfWidth = m_Camera ? m_Camera->GetViewportHalfWidth() : 640.0f;
    float viewportHalfHeight = m_Camera ? m_Camera->GetViewportHalfHeight() : 360.0f;

    // 預設邊界：沒有橫幅時退回視窗高度範圍
    float topLimit = cameraPos.y + viewportHalfHeight - 20.0f;
    float bottomLimit = cameraPos.y - viewportHalfHeight + 20.0f;

    // 尋找場上目前的上橫幅與下橫幅，取其底邊／頂邊作為生成範圍邊界
    for (const auto& mapObj : m_MapObjects) {
        if (mapObj->GetType() == MapObject::Type::BannerTop) {
            const float bottomEdge = mapObj->m_Transform.translation.y - mapObj->GetHalfHeight();
            topLimit = bottomEdge;
        } else if (mapObj->GetType() == MapObject::Type::BannerBottom) {
            const float topEdge = mapObj->m_Transform.translation.y + mapObj->GetHalfHeight();
            bottomLimit = topEdge;
        }
    }

    // 避免上下限顛倒（例如橫幅尺寸異常時）
    if (bottomLimit > topLimit) {
        std::swap(bottomLimit, topLimit);
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disY(bottomLimit, topLimit);
    
    float spawnX = cameraPos.x + viewportHalfWidth + 20.0f;
    float spawnY = disY(gen);
    
    return glm::vec2{spawnX, spawnY};
}

glm::vec2 App::GetCameraPos() const {
    return m_Camera ? m_Camera->GetCameraPos() : glm::vec2{0.0f, 0.0f};
}

float App::GetCameraViewportHalfWidth() const {
    return m_Camera ? m_Camera->GetViewportHalfWidth() : 640.0f;
}

float App::GetCameraViewportHalfHeight() const {
    return m_Camera ? m_Camera->GetViewportHalfHeight() : 360.0f;
}

int App::GetCharacterScore() const {
    return m_Character ? m_Character->GetScore() : 0;
}

float App::GetStageElapsedSeconds() const {
    return m_StageElapsedSeconds;
}

void App::ResetStageElapsedSeconds() {
    m_StageElapsedSeconds = 0.0f;
}

bool App::IsBossDefeated() const {
    return m_BossDefeated;
}

bool App::IsBossActive() const {
    return m_BossSpawned && static_cast<bool>(m_Boss);
}

void App::ResetBossState() {
    m_BossSpawned = false;
    m_BossDefeated = false;
    LOG_INFO("Boss state reset for new stage");
}

void App::UpdateHud() {
    if (!m_HudVisible || !m_Camera) {
        return;
    }

    const glm::vec2 cameraPos = m_Camera->GetCameraPos();
    const float viewportHalfWidth = m_Camera->GetViewportHalfWidth();
    const float viewportHalfHeight = m_Camera->GetViewportHalfHeight();

    // HUD 底條的世界座標 = 相機目前位置 + 固定的螢幕內偏移，確保貼齊螢幕底部、不受相機左右移動影響
    const float hudBaseY = cameraPos.y - viewportHalfHeight + 24.0f;   // 底部留 24px 邊距
    const float hudLeftX = cameraPos.x - viewportHalfWidth + 16.0f;   // 左側留 16px 邊距
    const float hudRightEdgeX = cameraPos.x + viewportHalfWidth - 16.0f; // 右側留 16px 邊距

    // --- 左側：生命值與分數文字內容更新（僅在數值改變時才重新產生文字貼圖）---
    const int health = m_Character ? m_Character->GetHealth() : 0;
    const int score = m_Character ? m_Character->GetScore() : 0;

    if (health != m_HudLastDisplayedHealth && m_HudLifeText) {
        m_HudLifeText->SetText("1P  HP:" + std::to_string(health));
        m_HudLastDisplayedHealth = health;
    }
    if (score != m_HudLastDisplayedScore && m_HudScoreText) {
        // 補零至 7 位數，跟原圖排版一致（例如 0000500）
        std::string scoreStr = std::to_string(score);
        if (scoreStr.size() < 7) {
            scoreStr = std::string(7 - scoreStr.size(), '0') + scoreStr;
        }
        m_HudScoreText->SetText("1P" + scoreStr);
        m_HudLastDisplayedScore = score;
    }

    m_HudLifeLabel->m_Transform.translation = {hudLeftX + 70.0f, hudBaseY + 18.0f};
    m_HudScoreLabel->m_Transform.translation = {hudLeftX + 70.0f, hudBaseY};

    // --- 右側：強化選項格，緊靠右邊緣由右往左排列 ---
    // 每格的版位寬度取「背景寬度」與「文字實際寬度」兩者中較大值，
    // 避免文字字串過長（例如 IMMUNITY）時寬度超過背景，導致與鄰格重疊。
    constexpr float kHudCellSpacing = 6.0f; // 格與格之間的固定間距
    float currentRightX = hudRightEdgeX;
    const int currentCapsuleCount = m_Character ? m_Character->GetCapsuleCount() : 0;

    for (int i = kHudPowerCellCount - 1; i >= 0; --i) {
        auto& background = m_HudPowerCellBackgrounds[i];
        auto& label = m_HudPowerCellLabels[i];

        const float backgroundWidth = background->GetScaledSize().x;
        const float labelWidth = label->GetScaledSize().x;
        const float cellWidth = std::max(backgroundWidth, labelWidth);

        const float cellCenterX = currentRightX - cellWidth * 0.5f;

        background->m_Transform.translation = {cellCenterX, hudBaseY};
        label->m_Transform.translation = {cellCenterX, hudBaseY};

        // 反白規則：目前持有的膠囊數對應到第幾格（1~5），該格整格反白
        const bool isActive = (currentCapsuleCount == i + 1);
        background->SetVisible(isActive);

        currentRightX -= (cellWidth + kHudCellSpacing);
    }
}

void App::ShowHud() {
    if (!m_HudInitialized) {
        InitHud();
    }
    m_HudVisible = true;

    m_HudLifeLabel->SetVisible(true);
    m_HudScoreLabel->SetVisible(true);
    for (auto& background : m_HudPowerCellBackgrounds) {
        background->SetVisible(true);
    }
    for (auto& label : m_HudPowerCellLabels) {
        label->SetVisible(true);
    }
}

void App::HideHud() {
    m_HudVisible = false;

    if (m_HudLifeLabel) m_HudLifeLabel->SetVisible(false);
    if (m_HudScoreLabel) m_HudScoreLabel->SetVisible(false);
    for (auto& background : m_HudPowerCellBackgrounds) {
        if (background) background->SetVisible(false);
    }
    for (auto& label : m_HudPowerCellLabels) {
        if (label) label->SetVisible(false);
    }
}

void App::ShowCenterMessage(const std::string& text) {
    if (!m_CenterMessageInitialized) {
        m_CenterMessageText = std::make_shared<Util::Text>(kHudFontPath, 32, text, Util::Color(255, 255, 255));
        m_CenterMessageLabel = std::make_shared<Util::GameObject>(m_CenterMessageText, kHudZIndex);
        m_Root.AddChild(m_CenterMessageLabel);
        m_CenterMessageInitialized = true;
    } else if (m_CenterMessageText) {
        m_CenterMessageText->SetText(text);
    }

    // 固定顯示在相機目前視野的正中央
    const glm::vec2 cameraPos = m_Camera ? m_Camera->GetCameraPos() : glm::vec2{0.0f, 0.0f};
    m_CenterMessageLabel->m_Transform.translation = cameraPos;
    m_CenterMessageLabel->SetVisible(true);
}

void App::HideCenterMessage() {
    if (m_CenterMessageLabel) {
        m_CenterMessageLabel->SetVisible(false);
    }
}

void App::SpawnBoss(std::shared_ptr<Boss> boss) {
    if (!boss || m_BossSpawned) {
        return;
    }

    // 切換為該 Boss 指定的戰場背景（會自動清除舊背景對應的地圖橫幅）
    SetBackgroundImage(boss->GetBackgroundPath());

    // Boss 戰期間不使用地圖橫幅：清空路徑設定，避免 Update() 的橫幅串流邏輯
    // 在下一幀又把剛清除的橫幅重新鋪回畫面上。
    ClearBannerPaths();

    // 清除場上所有敵人（不論是否在視野內），子彈/飛彈/膠囊保留不受影響
    for (auto& enemy : m_Enemies) {
        m_Root.RemoveChild(enemy);
    }
    m_Enemies.clear();

    m_Boss = std::move(boss);
    m_Root.AddChild(m_Boss);
    m_BossSpawned = true;
    m_BossDefeated = false;
    LOG_INFO("Boss spawned at ({}, {})", m_Boss->m_Transform.translation.x, m_Boss->m_Transform.translation.y);
}

void App::CreateCapsule(const glm::vec2& position) {
    auto capsule = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(RESOURCE_DIR "/Image/capsule.png"), 0.0f
    );
    capsule->m_Transform.translation = position;
    capsule->m_Transform.scale = {3.0f, 3.0f};
    m_Capsules.push_back(capsule);
    m_Root.AddChild(capsule);
}

void App::ClearSceneObjects() {
    for (auto& bullet : m_Bullets) m_Root.RemoveChild(bullet);
    for (auto& missile : m_Missiles) m_Root.RemoveChild(missile);
    for (auto& enemy : m_Enemies) m_Root.RemoveChild(enemy);
    for (auto& capsule : m_Capsules) m_Root.RemoveChild(capsule);
    for (auto& mapObj : m_MapObjects) m_Root.RemoveChild(mapObj);
    for (auto& fireball : m_Fireballs) m_Root.RemoveChild(fireball);
    
    m_Bullets.clear();
    m_Missiles.clear();
    m_Enemies.clear();
    m_Capsules.clear();
    m_MapObjects.clear();
    m_Fireballs.clear();
    
    if (m_Boss) {
        m_Root.RemoveChild(m_Boss);
        m_Boss.reset();
    }
    m_BossSpawned = false;
    m_BossDefeated = false;

    if (m_Character) {
        m_Root.RemoveChild(m_Character);
        m_Character.reset();
    }
}

void App::CreateMapObject(const glm::vec2& position, const std::string& relativePath) {
    auto image = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + relativePath);
    auto mapObj = std::make_shared<MapObject>(image, 0.0f);

    if (m_Camera) {
        const float viewportHeight = m_Camera->GetViewportHalfHeight() * 2.0f;
        const auto imageSize = image->GetSize();
        if (imageSize.y > 0.0f) {
            const float scale = viewportHeight / imageSize.y;
            mapObj->m_Transform.scale = {scale, scale};
        }
    }

    mapObj->m_Transform.translation = position;
    m_Root.AddChild(mapObj);
    m_MapObjects.push_back(mapObj);
    LOG_INFO("Map tile spawned at ({}, {})", position.x, position.y);
}

void App::CreateMapBanner(const std::string& relativePath, bool top) {
    if (!m_Camera) {
        return;
    }
    const auto cameraPos = m_Camera->GetCameraPos();
    const float viewportHalfWidth = m_Camera->GetViewportHalfWidth();
    const float rightEdge = cameraPos.x + viewportHalfWidth;
    CreateMapBannerAtLeft(relativePath, top, rightEdge);
}

std::shared_ptr<MapObject> App::CreateMapBannerAtLeft(const std::string& relativePath, bool top, float leftEdge) {
    auto image = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + relativePath);
    auto mapObj = std::make_shared<MapObject>(image, 0.0f, top ? MapObject::Type::BannerTop : MapObject::Type::BannerBottom);
    mapObj->m_Transform.scale = {3.0f, 3.0f};

    const float halfWidth = mapObj->GetHalfWidth();
    const float halfHeight = mapObj->GetHalfHeight();
    const float yPos = top ? (m_Camera->GetCameraPos().y + m_Camera->GetViewportHalfHeight() - halfHeight)
                          : (m_Camera->GetCameraPos().y - m_Camera->GetViewportHalfHeight() + halfHeight * 4.5f);
    mapObj->m_Transform.translation = {leftEdge + halfWidth, yPos};
    m_Root.AddChild(mapObj);
    m_MapObjects.push_back(mapObj);
    LOG_INFO("Banner tile spawned: {} at ({}, {})", relativePath, mapObj->m_Transform.translation.x, mapObj->m_Transform.translation.y);
    return mapObj;
}

void App::ClearMapBanners() {
    auto it = m_MapObjects.begin();
    while (it != m_MapObjects.end()) {
        if ((*it)->GetType() == MapObject::Type::BannerTop ||
            (*it)->GetType() == MapObject::Type::BannerBottom) {
            m_Root.RemoveChild(*it);
            it = m_MapObjects.erase(it);
        } else {
            ++it;
        }
    }
}

void App::HandleCharacterHit() {
    if (!m_Character) {
        return;
    }

    // 碰撞免疫優先判定：有免疫次數時，只消耗免疫次數，不扣血、不重生、不清膠囊
    if (m_Character->ConsumeCollisionImmunity()) {
        LOG_INFO("Character hit absorbed by collision immunity! Remaining immunity: {}",
            m_Character->GetCollisionImmunityCount());
        return;
    }

    m_Character->DecreaseHealth();
    LOG_INFO("Character Hit! Health: {}", m_Character->GetHealth());

    if (m_Character->GetHealth() <= 0) {
        LOG_INFO("Game Over! Final Score: {}, Kills: {}", m_Character->GetScore(), m_Character->GetKills());

        m_IsGameOver = true;
        m_IsCharacterRespawning = false;
        m_CharacterRespawnCounter = 0;
        m_GameOverInputReady = false;

        HideHud();
        ClearSceneObjects();
        SetBackgroundImage("/Image/gameover.png");
        return;
    }

    // 尚有剩餘生命：保留分數，膠囊數歸零，重生於畫面正中心
    LOG_INFO("Character lost a life, respawning at center... Remaining Health: {}", m_Character->GetHealth());

    m_RespawnHealth = m_Character->GetHealth();
    m_RespawnScore = m_Character->GetScore();
    m_RespawnKills = m_Character->GetKills();
    m_Character->SetCapsuleCount(0);

    m_Root.RemoveChild(m_Character);
    m_Character.reset();
    m_IsCharacterRespawning = true;
    m_CharacterRespawnCounter = 0;
}

void App::HandleGameOverInput() {
    // 空白鍵：在 gameover.png 與 gameover_end.png 之間切換
    if (Util::Input::IsKeyUp(Util::Keycode::SPACE)) {
        if (!m_IsGameOverEndScreen) {
            m_IsGameOverEndScreen = true;
            SetBackgroundImage("/Image/gameover_end.png");
            LOG_INFO("Game Over: switched to end screen");
        } else {
            m_IsGameOverEndScreen = false;
            SetBackgroundImage("/Image/gameover.png");
            LOG_INFO("Game Over: switched back to gameover screen");
        }
    }

    // F 鍵：依目前所在畫面決定行為
    if (Util::Input::IsKeyUp(Util::Keycode::F)) {
        if (!m_IsGameOverEndScreen) {
            // 背景為 gameover.png：從當前 Stage 重新開始
            RestartCurrentStage();
        } else {
            // 背景為 gameover_end.png：返回最初封面，等待 Enter 重新開始
            ReturnToFrontScreen();
        }
    }
}

void App::RestartCurrentStage() {
    LOG_INFO("Game Over: restarting current stage");

    const std::string stageName = m_StageManager ? m_StageManager->GetCurrentStageName() : std::string{};

    // 清空分數、膠囊、強化（重生時保留用的暫存值也一併歸零，避免下一次受傷時誤帶入舊數值）
    m_RespawnHealth = 2;
    m_RespawnScore = 0;
    m_RespawnKills = 0;

    m_IsGameOver = false;
    m_IsGameOverEndScreen = false;
    m_GameOverInputReady = false;
    m_IsCharacterRespawning = false;
    m_CharacterRespawnCounter = 0;
    m_IsCharacterCollisionDisabled = false;
    m_CharacterCollisionDisableCounter = 0;
    m_StageElapsedSeconds = 0.0f;
    m_HudLastDisplayedHealth = -1;
    m_HudLastDisplayedScore = -1;

    if (!stageName.empty() && m_StageManager) {
        m_StageManager->StartStage(stageName); // 重新呼叫 Stage::Enter()，會重新生成角色、重設分數等
    }
}

void App::ReturnToFrontScreen() {
    LOG_INFO("Game Over: returning to front screen");

    m_IsGameOver = false;
    m_IsGameOverEndScreen = false;
    m_GameOverInputReady = false;
    m_IsCharacterRespawning = false;
    m_CharacterRespawnCounter = 0;
    m_IsCharacterCollisionDisabled = false;
    m_CharacterCollisionDisableCounter = 0;
    m_StageElapsedSeconds = 0.0f;

    m_RespawnHealth = 2;
    m_RespawnScore = 0;
    m_RespawnKills = 0;

    // 結束目前的 Stage，讓 HasActiveStage() 回到 false，
    // 之後在 Update() 開頭才能再次偵測到 Enter 鍵並開始新一輪 Stage 1-1
    if (m_StageManager) {
        m_StageManager->StopStage();
    }

    // 回到尚未開始任何關卡的狀態（與 App::Start() 剛執行完一致），等待按 Enter 開始 Stage 1-1
    SetBackgroundImage("/Image/front.png");
}

void App::EnterEndingScreen() {
    LOG_INFO("Entering ending screen. Final score: {}", GetCharacterScore());

    m_IsEnding = true;
    m_EndingInputReady = false;

    // 結算階段視覺呈現：背景固定為 bossfield.png，隱藏 HUD，清空場上殘留物件
    HideHud();
    const int finalScore = GetCharacterScore();
    ClearSceneObjects();
    SetBackgroundImage("/Image/bossfield.png");
    ShowCenterMessage("SCORE: " + std::to_string(finalScore));
}

void App::HandleEndingInput() {
    if (Util::Input::IsKeyUp(Util::Keycode::SPACE)) {
        HideCenterMessage();
        m_IsEnding = false;
        m_EndingInputReady = false;
        ReturnToFrontScreen();
    }
}