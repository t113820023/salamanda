#ifndef ENEMY_BASE_HPP
#define ENEMY_BASE_HPP

#include "Util/GameObject.hpp"
#include <glm/glm.hpp>

/**
 * @class EnemyBase
 * @brief 所有「一般敵人」的共同基底類別（不含 Boss）。
 *
 * App 端的生成流程、碰撞判定（Bullet/Missile/Character vs Enemy）、
 * 離開螢幕移除、死亡查詢，全部透過這個介面操作，不綁定任何特定敵人的型別。
 * 新增一種敵人時，只需繼承這個類別並實作各虛擬函式即可，
 * 不需要更動 App.cpp 既有的碰撞與移除邏輯。
 *
 * @note Update() 統一要求傳入角色目前位置，讓需要追蹤角色的敵人（例如
 *       Enemy2）可以取得目標座標；不需要這個資訊的敵人（例如
 *       單純向左飛行的 Enemy）可以直接忽略此參數。
 */
class EnemyBase : public Util::GameObject {
public:
    virtual ~EnemyBase() = default;

    virtual void Update(const glm::vec2& characterPos) = 0;

    virtual bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const = 0;

    virtual int GetHealth() const = 0;
    virtual void TakeDamage(int damage) = 0;
    virtual bool IsDead() const = 0;
};

#endif // ENEMY_BASE_HPP