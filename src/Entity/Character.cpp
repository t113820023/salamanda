#include "Entity/Character.hpp"

#include "Entity/LaserBullet.hpp"
#include "Entity/SoundBullet.hpp"
#include "Entity/Missile.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"
#include "Util/Image.hpp"
#include "Util/Logger.hpp"

#include <glm/glm.hpp>
#include <memory>

Character::Character() {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/main_jet.png"));
    m_Transform.scale = {3.0f, 3.0f};
    SetZIndex(1.0f); // z-index 1.0，最前方
}

void Character::Update() {
    constexpr float kMoveSpeedUpgrade = 80.0F;

    glm::vec2 direction{0.0F, 0.0F};
    if (Util::Input::IsKeyPressed(Util::Keycode::W)) {
        direction.y += 1.0F;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) {
        direction.y -= 1.0F;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) {
        direction.x -= 1.0F;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) {
        direction.x += 1.0F;
    }

    if (direction != glm::vec2{0.0F, 0.0F}) {
        direction = glm::normalize(direction);
        const float deltaSeconds = Util::Time::GetDeltaTimeMs() / 1000.0F;
        m_Transform.translation += direction * (m_MoveSpeed * deltaSeconds);
    }

    // 按下 F 升級：膠囊數量決定升級類型
    if (Util::Input::IsKeyUp(Util::Keycode::F)) {
        if (m_capsuleCount == 1) {
            // 速度強化最多累積 kMaxSpeedUpgradeLevel 段，滿了之後不消耗膠囊、不做任何事
            if (m_SpeedUpgradeLevel < kMaxSpeedUpgradeLevel) {
                m_MoveSpeed += kMoveSpeedUpgrade;
                m_SpeedUpgradeLevel++;
                m_capsuleCount = 0;
                LOG_INFO("Character move speed upgraded to {} (level {}/{})",
                    m_MoveSpeed, m_SpeedUpgradeLevel, kMaxSpeedUpgradeLevel);
            } else {
                LOG_INFO("Character speed upgrade already maxed ({}/{}), capsule not consumed",
                    m_SpeedUpgradeLevel, kMaxSpeedUpgradeLevel);
            }
        } else if (m_capsuleCount == 2) {
            // 飛彈強化只能成功一次，已強化過則不消耗膠囊、不做任何事
            if (!m_MissileUpgradeActive) {
                m_MissileUpgradeActive = true;
                m_capsuleCount = 0;
                LOG_INFO("Character missile upgrade activated");
            } else {
                LOG_INFO("Character missile upgrade already active, capsule not consumed");
            }
        } else if (m_capsuleCount == 3) {
            // 子彈類型獨立判斷，可來回切換
            m_CurrentBulletType = BulletType::SOUND;
            m_capsuleCount = 0;
            LOG_INFO("Character bullet upgraded to SOUND");
        } else if (m_capsuleCount == 4) {
            m_CurrentBulletType = BulletType::LASER;
            m_capsuleCount = 0;
            LOG_INFO("Character bullet upgraded to LASER");
        } else if (m_capsuleCount == 5) {
            // 取得 5 次碰撞免疫（可疊加）
            constexpr int kImmunityGain = 5;
            m_CollisionImmunityCount += kImmunityGain;
            m_capsuleCount = 0;
            LOG_INFO("Character gained {} collision immunity (total {})",
                kImmunityGain, m_CollisionImmunityCount);
        }
    }

    // 更新連發冷卻
    if (m_BurstCooldown > 0) {
        m_BurstCooldown--;
    }

    // 更新主冷卻（連發完成後的總冷卻）
    if (m_ShootCooldown > 0) {
        m_ShootCooldown--;
    }

    // 3 連發射擊邏輯
    auto SpawnBullet = [this]() {
        std::shared_ptr<Bullet> bullet;
        switch (m_CurrentBulletType) {
            case BulletType::LASER:
                bullet = std::make_shared<LaserBullet>();
                break;
            case BulletType::SOUND:
                bullet = std::make_shared<SoundBullet>();
                break;
            case BulletType::NORMAL:
            default:
                bullet = std::make_shared<Bullet>();
                break;
        }

        const glm::vec2 characterSize = GetScaledSize();
        const glm::vec2 bulletSize = bullet->GetScaledSize();
        
        if (m_CurrentBulletType == BulletType::SOUND) {
            bullet->m_Transform.translation = m_Transform.translation + glm::vec2{characterSize.x / 2.0f, 0.0f};
        }
        else {
            bullet->m_Transform.translation = m_Transform.translation + glm::vec2{characterSize.x, characterSize.y * 0.5f} - glm::vec2{30.0f, bulletSize.y * 2.5f};
        }
        m_NewBullets.push_back(bullet);
    };

    // 根據子彈類型獲取對應的射擊間隔和連發數量
    int burstDelay = Bullet::kBurstDelay;
    int cooldown = Bullet::kCooldown;
    int maxBurstCount = Bullet::kBurstCount;
    
    switch (m_CurrentBulletType) {
        case BulletType::LASER:
            burstDelay = LaserBullet::kBurstDelay;
            cooldown = LaserBullet::kCooldown;
            maxBurstCount = LaserBullet::kBurstCount;
            break;
        case BulletType::SOUND:
            burstDelay = SoundBullet::kBurstDelay;
            cooldown = SoundBullet::kCooldown;
            maxBurstCount = SoundBullet::kBurstCount;
            break;
        case BulletType::NORMAL:
        default:
            burstDelay = Bullet::kBurstDelay;
            cooldown = Bullet::kCooldown;
            maxBurstCount = Bullet::kBurstCount;
            break;
    }

    if (m_BurstCount == 0 && Util::Input::IsKeyPressed(Util::Keycode::SPACE) && m_ShootCooldown == 0) {
        SpawnBullet();
        
        // 飛彈升級激活時生成上下飛彈
        if (m_MissileUpgradeActive) {
            const glm::vec2 characterSize = GetScaledSize();
            
            // 上側飛彈
            auto missileTop = std::make_shared<Missile>();
            missileTop->m_Transform.translation = m_Transform.translation + glm::vec2{characterSize.x / 2.0f, characterSize.y};
            m_NewMissiles.push_back(missileTop);
            
            // 下側飛彈
            auto missileBottom = std::make_shared<Missile>();
            missileBottom->m_Transform.translation = m_Transform.translation + glm::vec2{characterSize.x / 2.0f, -characterSize.y};
            m_NewMissiles.push_back(missileBottom);
        }
        
        m_BurstCount = 1;
        m_BurstCooldown = burstDelay;
        m_ShootCooldown = cooldown;
    }

    if (m_BurstCount > 0 && m_BurstCooldown == 0) {
        if (m_BurstCount < maxBurstCount) {
            SpawnBullet();
            m_BurstCount++;
            m_BurstCooldown = (m_BurstCount < maxBurstCount) ? burstDelay : 0;
        }

        if (m_BurstCount == maxBurstCount) {
            m_BurstCount = 0;
        }
    }
}

std::vector<std::shared_ptr<Bullet>> Character::GetNewBullets() {
    auto bullets = m_NewBullets;
    m_NewBullets.clear();
    return bullets;
}

std::vector<std::shared_ptr<class Missile>> Character::GetNewMissiles() {
    auto missiles = m_NewMissiles;
    m_NewMissiles.clear();
    return missiles;
}