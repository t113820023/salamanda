#ifndef FIREBALL_HPP
#define FIREBALL_HPP

#include "Util/GameObject.hpp"
#include <glm/glm.hpp>

/**
 * @class Fireball
 * @brief Boss2 每隔一段時間發射的攻擊物。
 *
 * 從 Boss2 位置往左（-x 方向）飛行，速度與一般 Bullet 相同。
 * 只會因為「碰到角色」或「飛出畫面」而消失，不與玩家的子彈/飛彈互動
 * （玩家無法直接擊毀火球）。Boss2 是否能受到傷害，取決於場上是否還
 * 存在至少一顆自己發出的 Fireball——這個判斷由 App/Boss2 端透過
 * 「目前火球數量」查詢，Fireball 本身不需要知道這件事。
 */
class Fireball : public Util::GameObject {
public:
    static constexpr float kSpeed = 600.0f; // 與 Bullet::kDefaultSpeed 一致
    static constexpr int kDamage = 1;       // 撞到角色造成的傷害（與其他敵方碰撞傷害一致）

    explicit Fireball(glm::vec2 startPos);

    void Update();

    bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const {
        const float leftBound = cameraPos.x - viewportHalfWidth;
        const float rightBound = cameraPos.x + viewportHalfWidth;
        const float topBound = viewportHalfHeight;
        const float bottomBound = -viewportHalfHeight;

        return m_Transform.translation.x < leftBound ||
               m_Transform.translation.x > rightBound ||
               m_Transform.translation.y < bottomBound ||
               m_Transform.translation.y > topBound;
    }

    int GetDamage() const { return kDamage; }
};

#endif // FIREBALL_HPP