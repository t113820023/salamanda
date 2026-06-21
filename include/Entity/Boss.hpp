#ifndef BOSS_HPP
#define BOSS_HPP

#include "Util/GameObject.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Character;

/**
 * @class Boss
 * @brief 所有 Boss 的共同基底類別。
 *
 * App 端的生成流程、碰撞判定（Bullet/Missile/Character vs Boss）、
 * 死亡與血量查詢，全部透過這個介面操作，不綁定任何特定 Boss 的型別。
 * 新增一個 Boss 時，只需繼承這個類別並實作各虛擬函式即可，
 * 不需要更動 App.cpp 既有的生成與碰撞邏輯。
 */
class Boss : public Util::GameObject {
public:
    virtual ~Boss() = default;

    // 每幀更新（朝向角色移動、攻擊模式等，由各 Boss 自行決定行為）
    virtual void Update(const std::shared_ptr<Character>& character) = 0;

    // 是否超出畫面範圍（用於場景清理判斷，非必要時可回傳 false）
    virtual bool IsOffScreen(const glm::vec2& cameraPos, float viewportHalfWidth = 640.0f, float viewportHalfHeight = 360.0f) const = 0;

    // 血量與傷害
    virtual int GetHealth() const = 0;
    virtual void TakeDamage(int damage) = 0;
    virtual bool IsDead() const = 0;
    virtual int GetDamage() const = 0;

    // 生成此 Boss 時要切換的背景圖（相對路徑），由各 Boss 決定自己的戰場背景
    virtual std::string GetBackgroundPath() const = 0;

    // --- 附屬攻擊物通知（泛用機制）---
    // 部分 Boss 會定期生成附屬攻擊物（例如 Boss2 的火球）。Boss 自己決定何時該生成
    // （通常在 Update() 內部處理計時），App 端每幀查詢此旗標，若為 true 則在
    // Boss 目前位置（m_Transform.translation）生成對應的攻擊物，再呼叫
    // ClearPendingAttack() 重置旗標。不需要此機制的 Boss 可直接使用預設實作
    // （永遠回傳 false，App 不會做任何事）。
    virtual bool HasPendingAttack() const { return false; }
    virtual void ClearPendingAttack() {}

    // --- 受傷資格查詢（泛用機制）---
    // 部分 Boss 的受傷資格與自己發出的攻擊物存活狀態有關（例如 Boss2 只有
    // 火球還在場上時才能受傷）。App 每幀統計「目前場上由此 Boss 生成的攻擊物
    // 數量」後，透過 NotifyAttackObjectCount() 回報給 Boss；Boss 內部記住這個
    // 數量，CanBeDamaged() 依此決定是否允許 TakeDamage() 真正生效。
    // 不需要此機制的 Boss（例如 Boss1）使用預設實作：永遠可受傷，忽略回報的數量。
    virtual bool CanBeDamaged() const { return true; }
    virtual void NotifyAttackObjectCount(int count) { (void)count; }
};

#endif // BOSS_HPP