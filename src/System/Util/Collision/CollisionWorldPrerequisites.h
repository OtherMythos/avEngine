#pragma once

#include <cmath>

namespace AV{

    typedef unsigned int CollisionEntryId;
    typedef unsigned long long CollisionPackedResult;

    static const CollisionEntryId COLLISION_ENTRY_ID_INVALID = 0xFFFFFFFF;
    static const CollisionEntryId COLLISION_PACKED_RESULT_INVALID = 0xFFFFFFFFFFFFFFFF;

    inline bool checkCircleCollision(float x1, float y1, float rad1, float x2, float y2, float rad2){
        float a = x1 - x2;
        float b = y1 - y2;

        float c = sqrt(a*a + b*b);

        return c <= rad1 + rad2;
    }

}
