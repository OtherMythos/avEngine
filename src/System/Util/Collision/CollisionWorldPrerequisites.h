#pragma once

#include <cmath>
#include <algorithm>
#include "System/EnginePrerequisites.h"

namespace AV{

    typedef unsigned int CollisionEntryId;
    typedef unsigned long long CollisionPackedResult;

    static const CollisionEntryId COLLISION_ENTRY_ID_INVALID = 0xFFFFFFFF;
    static const CollisionPackedResult COLLISION_PACKED_RESULT_INVALID = 0xFFFFFFFFFFFFFFFF;

    enum class CollisionEntryType : uint32{
        either,
        sender,
        receiver
    };

    inline bool checkCircleCollision(float x1, float y1, float rad1, float x2, float y2, float rad2){
        float dx = x1 - x2;
        float dy = y1 - y2;
        float radiusSum = rad1 + rad2;

        return (dx * dx + dy * dy) <= (radiusSum * radiusSum);
    }

    inline bool checkCircleRectangleCollision(float cx, float cy, float radius, float rx, float ry, float rw, float rh){
        float closestX = std::max(rx, std::min(cx, rx + rw));
        float closestY = std::max(ry, std::min(cy, ry + rh));

        float dx = cx - closestX;
        float dy = cy - closestY;

        return (dx * dx + dy * dy) <= (radius * radius);
    }

    inline bool checkRectangleCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2){
        return !(x1 + w1 < x2 || x2 + w2 < x1 ||
                 y1 + h1 < y2 || y2 + h2 < y1);
    }


}
