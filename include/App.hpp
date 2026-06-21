#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Camera.hpp"
#include "Coordinate.hpp"
#include "Entity/Bullet.hpp"
#include "Entity/EnemyBase.hpp"
#include "Entity/Enemy.hpp"
#include "Entity/Boss.hpp"
#include "StageManager.hpp"
#include <vector>
#include <memory>
#include <array>

class MapObject;

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End(); 

    // --- 提供給 Stage 呼叫的公用 API ---
    void SpawnCharacter(int health = 2);
    void SetBackgroundImage(const std::string& relativePath);
    void SpawnEnemyAt(glm::vec2 position);
    void SpawnEnemy2At(glm::vec2 position); // 生成 Enemy2（firebird_swangup 圖，待機3秒後直線衝刺角色方向）
    glm::vec2 GetRandomEnemySpawnPosition() const;

    // 設定本關卡使用的地圖橫幅圖片路徑（上/下），空字串代表該關卡不使用橫幅。
    // 須在 Stage::Enter() 時呼叫；Stage::Exit() 時建議呼叫 ClearBannerPaths()。
    void SetBannerPaths(const std::string& topBannerPath, const std::string& bottomBannerPath);
    void ClearBannerPaths();

    // --- 查詢 API（供 Stage 自行判斷生成條件、計算生成位置）---
    glm::vec2 GetCameraPos() const;
    float GetCameraViewportHalfWidth() const;
    float GetCameraViewportHalfHeight() const;
    int GetCharacterScore() const;          // 角色目前分數（角色不存在時回傳 0）
    float GetStageElapsedSeconds() const;   // 關卡持續時間（秒）
    void ResetStageElapsedSeconds();        // 重置關卡持續時間為 0（應在 Stage::Enter() 時呼叫）

    // --- Boss 相關泛用 API（不綁定任何特定 Boss 型別）---
    // 由 Stage 自行判斷生成條件（分數、時間等），決定何時生成哪個 Boss，再呼叫此 API。
    void SpawnBoss(std::shared_ptr<Boss> boss);
    bool IsBossDefeated() const;  // Boss 是否已生成且已被擊敗
    bool IsBossActive() const;    // Boss 是否已生成且尚未被擊敗（用於暫停一般敵人出怪）
    void ResetBossState();        // 重置 Boss 狀態旗標（應在 Stage::Enter() 時呼叫，以便進入新關卡時可生成新 Boss）

    // --- HUD（底部資訊條）API ---
    // HUD 物件由 App 統一建立、儲存與每幀更新；Stage 只需在 Enter()/Exit() 呼叫顯示/隱藏。
    void ShowHud();
    void HideHud();

    // --- 畫面中央訊息（泛用 API，任何 Stage 都可使用）---
    void ShowCenterMessage(const std::string& text);
    void HideCenterMessage();

    // --- 結算階段（遊戲全域的「結束」節點，與 front 同等級，不屬於任何 Stage）---
    // Stage 偵測到通關條件達成時呼叫此方法，App 會切換到結算畫面（背景 bossfield.png、
    // 顯示最終分數、隱藏 HUD、清空場上殘留物件），並等待空白鍵返回封面。
    void EnterEndingScreen();

    // --- 靜態常數設定 ---
    static constexpr int kEnemyWaveInitialDelay = Enemy::kWaveInitialDelay;
    static constexpr int kEnemyBurstSpawnDelay = Enemy::kBurstSpawnDelay;
    static constexpr int kEnemyBurstSize = Enemy::kBurstSize;
    static constexpr int kEnemyBurstIntervalMin = Enemy::kBurstIntervalMin;
    static constexpr int kEnemyBurstIntervalMax = Enemy::kBurstIntervalMax;
    static constexpr int kCharacterCollisionDisableDuration = 180; // 無敵時間（幀數）
    static constexpr float kMapMoveSpeed = 200.0f; // 地圖移動速度
    static constexpr int kHudPowerCellCount = 5;   // HUD 強化選項格數量：SPEED/MISSILE/SOUND/LASER/IMMUNITY

private:
    void ValidTask();
    void CreateCapsule(const glm::vec2& position);
    void CreateMapObject(const glm::vec2& position, const std::string& relativePath = "/Image/map_tile.png");
    void CreateMapBanner(const std::string& relativePath, bool top);
    std::shared_ptr<MapObject> CreateMapBannerAtLeft(const std::string& relativePath, bool top, float leftEdge);
    void ClearSceneObjects();
    void ClearMapBanners();
    void HandleCharacterHit();

    // --- Game Over 階段專屬邏輯 ---
    void HandleGameOverInput();   // 處理 Game Over 畫面下的空白鍵/F 鍵輸入
    void RestartCurrentStage();   // 按 F（gameover.png）：清空分數/膠囊/強化，重新開始當前關卡
    void ReturnToFrontScreen();   // 按 F（gameover_end.png）：回到最初封面，等待 Enter

    // --- 結算階段專屬邏輯 ---
    void HandleEndingInput();     // 處理結算畫面下的空白鍵輸入（返回封面）

    // --- HUD（底部資訊條）---
    void InitHud();               // 建立所有 HUD 物件（僅執行一次，於 Start() 呼叫）
    void UpdateHud();             // 每幀更新 HUD 位置（貼齊螢幕底部）與內容（分數/血量/反白）

private:
    State m_CurrentState = State::START;
    Util::Renderer m_Root;

    // --- 遊戲物件清單 ---
    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<class Character> m_Character;
    std::unique_ptr<UGO::Graphics::Camera> m_Camera;
    
    std::vector<std::shared_ptr<Bullet>> m_Bullets;
    std::vector<std::shared_ptr<class Missile>> m_Missiles;
    std::vector<std::shared_ptr<EnemyBase>> m_Enemies;
    std::vector<std::shared_ptr<class Fireball>> m_Fireballs; // Boss2 發射的火球
    std::vector<std::shared_ptr<Util::GameObject>> m_Capsules;
    std::vector<std::shared_ptr<MapObject>> m_MapObjects;
    
    std::shared_ptr<Boss> m_Boss;          // 目前場上的 Boss（任何繼承 Boss 的型別都可指派）
    bool m_BossSpawned = false;            // 目前是否有 Boss 存在（生成後為 true，死亡或被清空時重置為 false）
    bool m_BossDefeated = false;           // 本次戰鬥的 Boss 是否已被擊敗（用於 Stage 判斷關卡完成，由 SpawnBoss() 重置）
    float m_StageElapsedSeconds = 0.0f;    // 關卡持續時間（秒），供 Stage 自行判斷生成條件使用

    // --- 角色與遊戲狀態相關變數 (補全) ---
    bool m_IsGameOver = false;
    bool m_IsGameOverEndScreen = false;    // Game Over 階段中，是否已切換到 gameover_end.png
    bool m_GameOverInputReady = false;     // 進入 Game Over 後，須先偵測到 F/SPACE 都放開過一次才為 true
    bool m_IsEnding = false;               // 是否處於結算階段（與 front 同等級，不屬於任何 Stage）
    bool m_EndingInputReady = false;       // 進入結算階段後，須先偵測到 SPACE 放開過一次才為 true
    bool m_IsCharacterRespawning = false;
    bool m_IsCharacterCollisionDisabled = false;
    int m_CharacterCollisionDisableCounter = 0;
    int m_CharacterRespawnCounter = 0;
    
    int m_RespawnHealth = 2;
    int m_RespawnScore = 0;
    int m_RespawnKills = 0;
    glm::vec2 m_LastEnemyPosition = {0.0f, 0.0f};

    std::string m_CurrentBackgroundPath = "/Image/front.png";
    std::string m_TopBannerPath;    // 本關卡上橫幅圖片路徑（空字串代表不使用橫幅）
    std::string m_BottomBannerPath; // 本關卡下橫幅圖片路徑（空字串代表不使用橫幅）

    // --- 關卡管理器 ---
    std::unique_ptr<StageManager> m_StageManager;

    // --- HUD（底部資訊條）---
    bool m_HudVisible = false;
    bool m_HudInitialized = false;

    std::shared_ptr<Util::GameObject> m_HudLifeLabel;   // 第一行："1P" + 生命值（文字形式）
    std::shared_ptr<Util::GameObject> m_HudScoreLabel;  // 第二行："1P" + 完整分數
    std::shared_ptr<Util::Text> m_HudLifeText;          // 指向上面同一個 Drawable，供 SetText 更新內容
    std::shared_ptr<Util::Text> m_HudScoreText;         // 指向上面同一個 Drawable，供 SetText 更新內容
    int m_HudLastDisplayedHealth = -1;  // 快取上次顯示的血量，避免每幀重複呼叫 SetText
    int m_HudLastDisplayedScore = -1;   // 快取上次顯示的分數，避免每幀重複呼叫 SetText

    std::array<std::shared_ptr<Util::GameObject>, kHudPowerCellCount> m_HudPowerCellBackgrounds; // 反白背景
    std::array<std::shared_ptr<Util::GameObject>, kHudPowerCellCount> m_HudPowerCellLabels;       // SPEED 等文字

    // --- 畫面中央訊息（泛用，任何 Stage 皆可用）---
    std::shared_ptr<Util::GameObject> m_CenterMessageLabel;
    std::shared_ptr<Util::Text> m_CenterMessageText;
    bool m_CenterMessageInitialized = false;
};

#endif