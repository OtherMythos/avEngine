#pragma once

#include "CollisionWorldPrerequisites.h"

namespace AV{
    class CollisionWorldObject{
    public:
        CollisionWorldObject();
        virtual ~CollisionWorldObject();

        virtual void processCollision() = 0;
        virtual CollisionEntryId addCollisionPoint(float x, float y, float radius) = 0;
        virtual CollisionEntryId removeCollisionPoint(CollisionEntryId id) = 0;
        virtual int getNumCollisions() = 0;
        virtual CollisionPackedResult getCollisionPairForIdx(unsigned int idx) = 0;
        virtual bool setPositionForPoint(CollisionEntryId entryId, float x, float y) = 0;
    };
}
