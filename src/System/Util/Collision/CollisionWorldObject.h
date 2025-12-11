#pragma once

#include "CollisionWorldPrerequisites.h"
#include "System/EnginePrerequisites.h"

namespace AV{
    class CollisionWorldObject{
    public:
        CollisionWorldObject(int worldId);
        virtual ~CollisionWorldObject();

        virtual void processCollision() = 0;
        virtual CollisionEntryId addCollisionPoint(float x, float y, float radius, uint8 mask=0xFF, CollisionEntryType collisionType=CollisionEntryType::either) = 0;
        virtual CollisionEntryId addCollisionRectangle(float x, float y, float width, float height, uint8 mask=0xFF, CollisionEntryType collisionType=CollisionEntryType::either) = 0;
        virtual CollisionEntryId addCollisionRotatedRectangle(float x, float y, float width, float height, float rotation, uint8 mask=0xFF, CollisionEntryType collisionType=CollisionEntryType::either) = 0;
        virtual bool checkCollisionPoint(float x, float y, float radius) = 0;
        virtual CollisionEntryId removeCollisionEntry(CollisionEntryId id) = 0;
        virtual int getNumCollisions() = 0;
        virtual CollisionPackedResult getCollisionPairForIdx(unsigned int idx) = 0;
        virtual bool setPositionForPoint(CollisionEntryId entryId, float x, float y) = 0;

    private:
        int mWorldId;

    protected:
        uint64 determineEnterLeaveBits(CollisionPackedResult first, CollisionPackedResult second, std::set<CollisionPackedResult>& prevPairs) const;
    };
}
