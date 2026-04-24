#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Transform.hpp"
#include "Bullet.hpp"
#include <vector>
#include <memory>

enum class BulletType {
    NORMAL = 0,   // 普通子彈
    SOUND = 1,    // 聲波
    LASER = 2     // 雷射
};

class Character : public Util::GameObject {
    public:
        Character();
        ~Character() = default;

        void Update();

        std::vector<std::shared_ptr<Bullet>> GetNewBullets();

        // 生命值相關
        int GetHealth() const { return m_Health; }
        void SetHealth(int health) { m_Health = health; }
        void DecreaseHealth(int amount = 1) { m_Health -= amount; }
        bool IsDead() const { return m_Health <= 0; }

        // 分數相關
        int GetScore() const { return m_Score; }
        void SetScore(int score) { m_Score = score; }
        void AddScore(int points) { m_Score += points; }

        // 擊殺數相關
        int GetKills() const { return m_Kills; }
        void SetKills(int kills) { m_Kills = kills; }
        void AddKill() { m_Kills += 1; }

        // 膠囊相關
        int GetCapsuleCount() const { return m_capsuleCount; }
        void SetCapsuleCount(int count) { m_capsuleCount = count; }
        void AddCapsuleCount(int amount = 1) { m_capsuleCount += amount; }

        // 子彈類型相關
        BulletType GetBulletType() const { return m_CurrentBulletType; }
        void SetBulletType(BulletType type) { m_CurrentBulletType = type; }

    private:
        std::vector<std::shared_ptr<Bullet>> m_NewBullets;
        int m_ShootCooldown = 0;  // 主冷卻時間（連發之間的間隔）
        int m_BurstCount = 0;  // 目前已發射的連發子彈數（0-3）
        int m_BurstCooldown = 0;  // 連發子彈之間的冷卻

        int m_Health = 2;  
        int m_Score = 0;   
        int m_Kills = 0;   
        int m_capsuleCount = 0;
        BulletType m_CurrentBulletType = BulletType::NORMAL;  // 當前子彈類型
};

#endif // CHARACTER_HPP