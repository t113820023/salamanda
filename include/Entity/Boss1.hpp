#ifndef BOSS1_HPP
#define BOSS1_HPP

#include "Entity/Boss.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Character;

class Boss1 : public Boss {
public:
    // Boss1 相關參數
    static constexpr int kSpawnScoreThreshold = 20000;   // 生成時所需的分數
    static constexpr float kSpawnTimeThreshold = 150.0f; // 生成時所需的關卡持續時間
    static constexpr int kHealth = 50;                   // Boss1 生命值
    static constexpr float kSpeed = 60.0f;               // Boss1 移動速度
    static constexpr int kDamage = 1;                    // Boss1 傷害值

    Boss1(glm::vec2 startPos);

    void Update(const std::shared_ptr<Character>& character) override;
    bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const override;

    int GetHealth() const override { return m_Health; }
    void TakeDamage(int damage) override { m_Health -= damage; }
    bool IsDead() const override { return m_Health <= 0; }
    int GetDamage() const override { return kDamage; }
    std::string GetBackgroundPath() const override { return "/Image/bossfield.png"; }

private:
    int m_Health = kHealth;
};

#endif // BOSS1_HPP