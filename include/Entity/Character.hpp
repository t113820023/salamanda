#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Transform.hpp"
#include "Bullet.hpp"
#include <vector>
#include <memory>
#include <algorithm>

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
        std::vector<std::shared_ptr<class Missile>> GetNewMissiles();

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

        // 膠囊持有上限
        static constexpr int kMaxCapsuleCount = 5;

        int GetCapsuleCount() const { return m_capsuleCount; }
        void SetCapsuleCount(int count) { m_capsuleCount = std::clamp(count, 0, kMaxCapsuleCount); }
        void AddCapsuleCount(int amount = 1) {
            m_capsuleCount = std::clamp(m_capsuleCount + amount, 0, kMaxCapsuleCount);
        }

        // 子彈類型相關
        BulletType GetBulletType() const { return m_CurrentBulletType; }
        void SetBulletType(BulletType type) { m_CurrentBulletType = type; }

        float GetMoveSpeed() const { return m_MoveSpeed; }
        void UpgradeMoveSpeed(float amount) { m_MoveSpeed += amount; }

        // 速度強化段數
        static constexpr int kMaxSpeedUpgradeLevel = 10;
        int GetSpeedUpgradeLevel() const { return m_SpeedUpgradeLevel; }

        // 碰撞免疫相關
        int GetCollisionImmunityCount() const { return m_CollisionImmunityCount; }
        void AddCollisionImmunity(int amount) { m_CollisionImmunityCount += amount; }
        bool ConsumeCollisionImmunity() {
            if (m_CollisionImmunityCount > 0) {
                m_CollisionImmunityCount--;
                return true;
            }
            return false;
        }

        // 子彈傷害提升（永久生效，直到角色重生/關卡重啟才恢復；建立新 Character 物件即自動恢復預設值）
        static constexpr int kBoostedBulletDamage = 30;
        bool IsBulletDamageBoostActive() const { return m_BulletDamageBoostActive; }
        void ActivateBulletDamageBoost() { m_BulletDamageBoostActive = true; }

    private:
        std::vector<std::shared_ptr<Bullet>> m_NewBullets;
        std::vector<std::shared_ptr<class Missile>> m_NewMissiles;
        int m_ShootCooldown = 0;  // 主冷卻時間（連發之間的間隔）
        int m_BurstCount = 0;  // 目前已發射的連發子彈數（0-3）
        int m_BurstCooldown = 0;  // 連發子彈之間的冷卻
        bool m_MissileUpgradeActive = false;  // 飛彈升級是否激活

        int m_Health = 2;  
        int m_Score = 0;   
        int m_Kills = 0;   
        int m_capsuleCount = 0;
        float m_MoveSpeed = 150.0F; // 角色當前移動速度
        BulletType m_CurrentBulletType = BulletType::NORMAL;  // 當前子彈類型

        int m_SpeedUpgradeLevel = 0;        // 已使用的速度強化次數（上限 kMaxSpeedUpgradeLevel）
        int m_CollisionImmunityCount = 0;   // 目前持有的碰撞免疫次數
        bool m_BulletDamageBoostActive = false; // 子彈傷害提升是否已啟用（永久生效直到重生/重啟）
};

#endif // CHARACTER_HPP