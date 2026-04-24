#include "Entity/Character.hpp"

#include "Entity/LaserBullet.hpp"
#include "Entity/SoundBullet.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"
#include "Util/Image.hpp"

#include <glm/glm.hpp>
#include <memory>

Character::Character() {
    SetDrawable(std::make_shared<Util::Image>(std::string(RESOURCE_DIR) + "/Image/main_jet.png"));
    m_Transform.scale = {3.0f, 3.0f};
    SetZIndex(1.0f); // z-index 1.0，最前方
}

void Character::Update() {
    constexpr float kMoveSpeed = 100.0F; // pixels per second

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
        m_Transform.translation += direction * (kMoveSpeed * deltaSeconds);
    }

    // 更新連發冷卻
    if (m_BurstCooldown > 0) {
        m_BurstCooldown--;
    }

    // 更新主冷卻（連發完成後的總冷卻）
    if (m_ShootCooldown > 0) {
        m_ShootCooldown--;
    }

    // 檢查F鍵，根據膠囊數量切換子彈類型
    if (Util::Input::IsKeyUp(Util::Keycode::F)) {
        if (m_capsuleCount == 3) {
            m_CurrentBulletType = BulletType::SOUND;  // 切換成聲波
            m_capsuleCount = 0;  // 清空膠囊計數
        } else if (m_capsuleCount == 4) {
            m_CurrentBulletType = BulletType::LASER;  // 切換成雷射
            m_capsuleCount = 0;  // 清空膠囊計數
        }
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
        bullet->m_Transform.translation = m_Transform.translation + glm::vec2{characterSize.x, characterSize.y * 0.5f} - glm::vec2{30.0f, bulletSize.y * 2.5f};
        m_NewBullets.push_back(bullet);
    };

    constexpr int kBurstDelay = 25;   // 連發間隔
    constexpr int kCooldown = 140;    // 連發完成後冷卻

    if (m_BurstCount == 0 && Util::Input::IsKeyPressed(Util::Keycode::SPACE) && m_ShootCooldown == 0) {
        SpawnBullet();
        m_BurstCount = 1;
        m_BurstCooldown = kBurstDelay;
        m_ShootCooldown = kCooldown;
    }

    if (m_BurstCount > 0 && m_BurstCooldown == 0) {
        if (m_BurstCount < 3) {
            SpawnBullet();
            m_BurstCount++;
            m_BurstCooldown = (m_BurstCount < 3) ? kBurstDelay : 0;
        }

        if (m_BurstCount == 3) {
            m_BurstCount = 0;
        }
    }
}

std::vector<std::shared_ptr<Bullet>> Character::GetNewBullets() {
    auto bullets = m_NewBullets;
    m_NewBullets.clear();
    return bullets;
}
