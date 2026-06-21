#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Camera.hpp"
#include "Coordinate.hpp"
#include "Entity/Bullet.hpp"
#include "Entity/Enemy.hpp"
#include "Entity/Boss1.hpp"
#include "StageManager.hpp"
#include <vector>
#include <memory>

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
    void End(); // NOLINT(readability-convert-member-functions-to-static)

    // --- 提供給關卡 (Stage) 呼叫的 API ---
    void SpawnCharacter(int health = 2);
    void SetBackgroundImage(const std::string& relativePath);
    void SpawnEnemyAt(glm::vec2 position);
    glm::vec2 GetRandomEnemySpawnPosition() const;
    bool IsBoss1Defeated() const;

private:
    State m_CurrentState = State::START;
    Util::Renderer m_Root;

    // --- 遊戲物件清單 ---
    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<class Character> m_Character;
    std::unique_ptr<Util::Camera> m_Camera;
    
    std::vector<std::shared_ptr<Bullet>> m_Bullets;
    std::vector<std::shared_ptr<class Missile>> m_Missiles;
    std::vector<std::shared_ptr<Enemy>> m_Enemies;
    std::vector<std::shared_ptr<Util::GameObject>> m_Capsules;
    std::vector<std::shared_ptr<MapObject>> m_MapObjects;
    
    std::shared_ptr<class Boss1> m_Boss1;
    bool m_Boss1Spawned = false;

    bool m_IsCharacterRespawning = false;

    // --- 關卡管理器 ---
    std::unique_ptr<StageManager> m_StageManager;
};

#endif