#pragma once

#include <vector>
#include <stack>
#include <set>

#include "CollisionWorldObject.h"
#include "CollisionWorldPrerequisites.h"

namespace AV{
    class CollisionWorldBruteForce : public CollisionWorldObject{
    public:
        CollisionWorldBruteForce();
        ~CollisionWorldBruteForce();

        void processCollision();
        CollisionEntryId addCollisionPoint(float x, float y, float radius, uint8 mask=0xFF, CollisionEntryType collisionType=CollisionEntryType::either);
        CollisionEntryId removeCollisionPoint(CollisionEntryId id);
        int getNumCollisions();
        CollisionPackedResult getCollisionPairForIdx(unsigned int idx);
        bool setPositionForPoint(CollisionEntryId idx, float x, float y);

    private:
        struct BruteForceEntry{
            float x;
            float y;
            float radius;
            uint8 mask;
            CollisionEntryType entryType = CollisionEntryType::either;
            bool hole = false;
        };

        std::vector<BruteForceEntry> mEntries;
        std::stack<size_t> mEntryHoles;
        std::set<CollisionEntryId> mDirtyPoints;
        std::vector<CollisionPackedResult> mCollisions;
    };
}