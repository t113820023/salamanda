#ifndef MAPOBJECT_HPP
#define MAPOBJECT_HPP

#include "pch.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

class MapObject : public Util::GameObject {
public:
    enum class Type {
        Generic,
        BannerTop,
        BannerBottom,
    };

    MapObject(const std::shared_ptr<Util::Image>& image, float zIndex = 0.0f, Type type = Type::Generic)
        : Util::GameObject(image, zIndex), m_Type(type) {}

    // 更新位置（以秒為單位的 delta 與速度）
    void Update(float deltaSeconds, float speed) {
        m_Transform.translation.x -= speed * deltaSeconds;
    }

    float GetHalfWidth() const {
        return GetScaledSize().x * 0.5f;
    }

    float GetHalfHeight() const {
        return GetScaledSize().y * 0.5f;
    }

    Type GetType() const noexcept {
        return m_Type;
    }

private:
    Type m_Type = Type::Generic;
};

#endif
