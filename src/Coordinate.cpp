#include "Coordinate.hpp"

namespace UGO {
namespace Core {
    GridPosition MapGridSize = {0, 0};
    Bounds WorldBounds = {0.0f, 0.0f, 0.0f, 0.0f};

    Bounds Bounds::FromCenter(float width, float height) {
        return {
            -width / 2.0f,
            -height / 2.0f,
            width / 2.0f,
            height / 2.0f
        };
    }

    WorldPosition GridToWorld(const GridPosition& gridPos) {
        return WorldPosition(gridPos.x * TILE_SIZE, gridPos.y * TILE_SIZE);
    }

    GridPosition WorldToGrid(const WorldPosition& worldPos) {
        return GridPosition(
            static_cast<int>(worldPos.x / TILE_SIZE),
            static_cast<int>(worldPos.y / TILE_SIZE)
        );
    }
} // namespace Core
} // namespace UGO