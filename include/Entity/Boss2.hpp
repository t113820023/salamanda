#ifndef BOSS2_HPP
#define BOSS2_HPP

#include "Entity/Boss.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Character;

/**
 * @class Boss2
 * @brief 固定在畫面右側、只做垂直追蹤移動的 Boss。
 *
 * 生成時位於畫面右側中心（X 軸鎖定，使圖片右緣緊貼螢幕右邊界），
 * 之後每幀只跟隨角色的 Y 座標移動，X 座標永遠不變。
 * 每隔 kFireballInterval 秒會透過 HasPendingAttack() 通知 App
 * 在自己目前位置生成一顆 Fireball。
 *
 * @note Boss2 本身的 TakeDamage() 沒有任何特殊邏輯（呼叫了就一定扣血）。
 *       「Boss2 是否處於可受傷狀態」由 App 端的碰撞判定負責：只有當場上
 *       還存在至少一顆由 Boss2 發出的 Fireball 時，App 才會呼叫
 *       Boss2::TakeDamage()；沒有火球存在時，子彈/飛彈打中 Boss2 不會
 *       觸發任何傷害判定。
 */
class Boss2 : public Boss {
public:
    static constexpr int kHealth = 50;                 
    static constexpr int kDamage = 1;                   
    static constexpr float kFireballInterval = 3.0f;     // 發射火球的間隔
    static constexpr int kSpawnScoreThreshold = 40000;   // 生成時所需的分數
    static constexpr float kSpawnTimeThreshold = 300.0f; // 生成時所需的關卡持續時間

    explicit Boss2(glm::vec2 startPos);

    void Update(const std::shared_ptr<Character>& character) override;
    bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const override;

    int GetHealth() const override { return m_Health; }
    void TakeDamage(int damage) override { m_Health -= damage; }
    bool IsDead() const override { return m_Health <= 0; }
    int GetDamage() const override { return kDamage; }
    std::string GetBackgroundPath() const override { return "/Image/bossfield.png"; }

    bool HasPendingAttack() const override { return m_HasPendingAttack; }
    void ClearPendingAttack() override { m_HasPendingAttack = false; }

    bool CanBeDamaged() const override { return m_ActiveFireballCount > 0; }
    void NotifyAttackObjectCount(int count) override { m_ActiveFireballCount = count; }

private:
    int m_Health = kHealth;
    float m_FireballTimer = 0.0f;
    bool m_HasPendingAttack = false;
    int m_ActiveFireballCount = 0; // 目前場上存活的、由自己發出的火球數量
};

#endif // BOSS2_HPP