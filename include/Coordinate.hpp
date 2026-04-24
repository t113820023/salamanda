#ifndef UGO_COORDINATE_HPP
#define UGO_COORDINATE_HPP

#include <glm/glm.hpp>

namespace UGO {
namespace Core {
    /* TODO[#13]: Change the name of global variable
    */
    using GridPosition = glm::ivec2;
    using WorldPosition = glm::vec2;

    using Direction = glm::vec2;
    using Distance = float;
    
    const int TILE_SIZE = 32;

    const int WindowHeight = 1280;
    const int WindowWidth = 720;

    
    struct Bounds { 
    float minX, minY;
    float maxX, maxY;

    static Bounds FromCenter(float width, float height);
    };

    extern GridPosition MapGridSize;
    extern Bounds WorldBounds;

    Core::WorldPosition GridToWorld(const Core::GridPosition& gridPos);
    Core::GridPosition WorldToGrid(const Core::WorldPosition &worldPos);
    
} // namespace Core
} // namespace UGO

#endif