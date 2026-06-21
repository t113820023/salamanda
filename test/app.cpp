#include "App.hpp"
#include "Entity.hpp"
#include "Entity/Boss1.hpp"
#include "Entity/Missile.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "Core/Context.hpp"
#include "StageManager.hpp"
#include <algorithm>
#include <limits>
#include <random>
#include "MapObject.hpp"

// 碰撞偵測 (保留你原本的實作)
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

    m_Camera = std::make_unique<Util::Camera>(100.0f);
    auto context = Core::Context::GetInstance();
    
    auto image = std::make_shared<Util::Image>(RESOURCE_DIR "/Image/front.png");
    m_Background = std::make_shared<Util::GameObject>(image, 0.0f);
    m_Background->m_Transform.scale = {1.0f, 1.0f};
    m_Background->m_Transform.translation = {0.0f, 0.0f}; 
    m_Root.AddChild(m_Background);

    // 初始化關卡管理器
    m_StageManager = std::make_unique<StageManager>(this);
}

void App::Update() {
    float deltaSeconds = Util::Time::GetDeltaTimeMs() / 1000.0f;
    
    // 1. 更新相機
    if (m_Camera) {
        m_Camera->Update(deltaSeconds);
    }

    // 2. 執行關卡邏輯 (交給導演 Stage1_1 去計算要不要出怪)
    if (m_StageManager) {
        m_StageManager->Update(deltaSeconds);
    }

    // 3. 更新所有遊戲物件的位置 (子彈、敵人、角色等)
    // ... 保留你原本更新 Entities 與碰撞偵測的迴圈邏輯 ...

    // 4. 繪製所有物件
    m_Root.Update();

    // 5. 結束判定
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}

// --- 提供給關卡的 API 實作 ---

void App::SetBackgroundImage(const std::string& relativePath) {
    auto newBackground = std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + relativePath);
    if (m_Background) {
        m_Background->SetDrawable(newBackground);
    }
}

void App::SpawnCharacter(int health) {
    if (!m_Character) {
        // ... 保留你原本生成 Character 的邏輯 ...
    }
}

void App::SpawnEnemyAt(glm::vec2 position) {
    auto enemy = std::make_shared<Enemy>();
    enemy->m_Transform.translation = position;
    m_Enemies.push_back(enemy);
    m_Root.AddChild(enemy);
}

glm::vec2 App::GetRandomEnemySpawnPosition() const {
    glm::vec2 cameraPos = m_Camera ? m_Camera->GetCameraPos() : glm::vec2{0.0f, 0.0f};
    float viewportHalfWidth = 128.0f; 
    float viewportHalfHeight = 120.0f;
    
    // 隨機生成在相機右側邊緣
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disY(cameraPos.y - viewportHalfHeight + 20.0f, 
                                              cameraPos.y + viewportHalfHeight - 20.0f);
    
    float spawnX = cameraPos.x + viewportHalfWidth + 20.0f;
    float spawnY = disY(gen);
    
    return glm::vec2{spawnX, spawnY};
}

bool App::IsBoss1Defeated() const {
    // 如果 Boss 存在且血量小於等於 0，或是你想設定的其他條件
    return m_Boss1Spawned && m_Boss1 && m_Boss1->IsDead();
}