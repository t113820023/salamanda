#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Camera.hpp"
#include "Coordinate.hpp"
#include "Entity/Bullet.hpp"
#include "Entity/Enemy.hpp"
#include <vector>

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

    void End(); // NOLINT(readability-convert-member-functions-to-static)

    static constexpr int kEnemyWaveInitialDelay = 30; // 開始遊戲後等待第一波生成的幀數
    static constexpr int kEnemyBurstSpawnDelay = 150;   // 一波內每隻敵人連續生成的間隔幀數
    static constexpr int kEnemyBurstSize = 5;          // 一波內的敵人數量
    static constexpr int kEnemyBurstIntervalMin = 120; // 每波之間最短等待幀數
    static constexpr int kEnemyBurstIntervalMax = 240; // 每波之間最長等待幀數

private:
    void ValidTask();
    void SpawnCharacter(int health = 2);
    void SpawnEnemyAt(const glm::vec2& position);
    void StartEnemyBurst();
    void CreateCapsule(const glm::vec2& position);
    void ClearSceneObjects();
    void SetBackgroundImage(const std::string& relativePath);

private:
    State m_CurrentState = State::START;

    Util::Renderer m_Root;
    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<class Character> m_Character;
    std::unique_ptr<UGO::Graphics::Camera> m_Camera;
    std::vector<std::shared_ptr<Bullet>> m_Bullets;
    std::vector<std::shared_ptr<Enemy>> m_Enemies;
    std::vector<std::shared_ptr<Util::GameObject>> m_Capsules;
    
    int m_EnemyBurstCount = 0;               // 本波已生成的敵人數量
    int m_EnemyBurstSpawnCounter = 0;         // 連續生成間隔計數
    int m_EnemyBurstInterval = 0;             // 波次之間的隨機間隔
    bool m_IsEnemyBurstActive = false;        // 是否正在生成敵人波
    bool m_IsWaitingForEnemyWave = false;     // 是否等待敵人波次開始
    int m_EnemyWaveWaitCounter = 0;           // 等待敵人波次開始的計數
    glm::vec2 m_EnemyBurstSpawnPosition = glm::vec2{0.0f, 0.0f}; // 本波敵人一致生成位置
    glm::vec2 m_LastEnemyPosition = glm::vec2{0.0f, 0.0f};  // 最後一個敵人的位置

    bool m_IsCharacterRespawning = false; // 是否正在重新生成角色
    int m_CharacterRespawnCounter = 0;    // 角色重新生成計數器
    int m_RespawnHealth = 0;             // 重生時保留的剩餘生命值
    int m_RespawnScore = 0;              // 重生時保留的分數
    int m_RespawnKills = 0;              // 重生時保留的擊殺數
    bool m_IsGameOver = false;           // 遊戲是否已經結束
};

#endif
