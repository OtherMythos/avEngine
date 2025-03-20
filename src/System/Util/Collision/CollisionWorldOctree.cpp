#include "CollisionWorldOctree.h"

#include <cassert>

namespace AV {

CollisionWorldOctree::CollisionWorldOctree(int worldId, const AABB& bounds, int maxDepth)
    : CollisionWorldObject(worldId), mRoot(bounds, maxDepth) {}

CollisionWorldOctree::~CollisionWorldOctree() {}

void CollisionWorldOctree::processCollision() {
    mCollisions.clear();
    std::vector<CollisionEntry*> potentialCollisions;
    std::set<CollisionPackedResult> pairs;
    std::set<CollisionPackedResult> enterLeavePairs;

    for (CollisionEntry& entry : mEntries) {
        if(entry.hole){
            if(entry.dirtyHole){
                entry.dirtyHole = false;
                mEntryHoles.push(entry.id);
            }
            continue;
        }

        if(entry.entryType == CollisionEntryType::receiver) continue;
        potentialCollisions.clear();
        mRoot.query(entry, mEntries, potentialCollisions);


        for (CollisionEntry* other : potentialCollisions) {
            if(entry.id == other->id) continue;
            if((other->mask & entry.mask) == 0) continue;

            uint64 targetFirstId = entry.id;
            uint64 targetSecondId = other->id;
            if(other->entryType != CollisionEntryType::either || entry.entryType != CollisionEntryType::either){
                if(
                   other->entryType == CollisionEntryType::sender && entry.entryType == CollisionEntryType::receiver)
                {
                    targetFirstId = other->id;
                    targetSecondId = entry.id;
                }
                else if(other->entryType == CollisionEntryType::receiver && entry.entryType == CollisionEntryType::sender)
                {
                    targetFirstId = entry.id;
                    targetSecondId = other->id;
                }else{
                   //Just ignore it if it does't match up correctly.
                   continue;
                }
            }

            if (&entry != other && checkCollision_(entry, *other)) {
                CollisionPackedResult first = static_cast<uint64>(targetFirstId) << 30 | targetSecondId;
                CollisionPackedResult second = static_cast<uint64>(targetSecondId) << 30 | targetFirstId;
                if(pairs.find(first) == pairs.end() && pairs.find(second) == pairs.end()){
                    CollisionPackedResult outResult = second;
                    enterLeavePairs.insert(outResult);
                    outResult |= CollisionWorldObject::determineEnterLeaveBits(first, second, mPrevPairs) << 60;
                    mCollisions.emplace_back(outResult);
                }

                pairs.insert(first);
                pairs.insert(second);
            }
        }
    }

    for(CollisionPackedResult i : mPrevEnterLeavePairs){
        if(enterLeavePairs.find(i) != enterLeavePairs.end()) continue;
        //Register that this point left.
        CollisionPackedResult out = i | (static_cast<CollisionPackedResult>(0x2) << 60);
        mCollisions.push_back(out);
    }

    mPrevPairs = enterLeavePairs;
    mPrevEnterLeavePairs = enterLeavePairs;
}

CollisionEntryId CollisionWorldOctree::addCollisionPoint(float x, float y, float radius, uint8 mask, CollisionEntryType type) {
    CollisionEntry entry(CollisionShape::CIRCLE, x, y, mask, type, radius, 0, 0);
    //entry.id = static_cast<CollisionEntryId>(mEntries.size());
    //mEntries.push_back(entry);

    CollisionEntryId added = addEntry_(entry);
    mRoot.insert(entry.id, mEntries, entry);
    return added;
}

CollisionEntryId CollisionWorldOctree::addCollisionRectangle(float x, float y, float width, float height, uint8 mask, CollisionEntryType type) {
    CollisionEntry entry(CollisionShape::RECTANGLE, x, y, mask, type, 0, width, height);
    entry.r.width = width;
    entry.r.height = height;
    CollisionEntryId added = addEntry_(entry);
    mRoot.insert(entry.id, mEntries, entry);
    return added;
}

CollisionEntryId CollisionWorldOctree::removeCollisionEntry(CollisionEntryId id) {
    if (id >= mEntries.size() || mEntries[id].hole) return false;
    mRoot.remove(id, mEntries[id]);
    mEntries[id].hole = true;
    mEntries[id].dirtyHole = true;

    return 0;
}

CollisionEntryId CollisionWorldOctree::addEntry_(CollisionEntry& entry){
    CollisionEntryId targetIdx = COLLISION_ENTRY_ID_INVALID;
    if(mEntryHoles.empty()){
        size_t id = mEntries.size();
        targetIdx = static_cast<CollisionEntryId>(id);
        entry.id = targetIdx;
        mEntries.push_back(entry);
    }else{
        size_t idx = mEntryHoles.top();
        mEntryHoles.pop();
        assert(mEntries[idx].hole == true);
        assert(mEntries[idx].dirtyHole == false);
        targetIdx = static_cast<CollisionEntryId>(idx);
        entry.id = targetIdx;
        mEntries[idx] = entry;
    }
    assert(targetIdx != COLLISION_ENTRY_ID_INVALID);

    return targetIdx;

}

bool CollisionWorldOctree::checkCollisionPoint(float x, float y, float radius) {
    CollisionEntry query(CollisionShape::CIRCLE, x, y, 0xFF, CollisionEntryType::either, radius, 0, 0);
    std::vector<CollisionEntry*> potentialCollisions;
    mRoot.query(query, mEntries, potentialCollisions);

    for (const auto* entry : potentialCollisions) {
        if (checkCollision_(query, *entry)) {
            return true;
        }
    }
    return false;
}

int CollisionWorldOctree::getNumCollisions() {
    return static_cast<int>(mCollisions.size());
}

CollisionPackedResult CollisionWorldOctree::getCollisionPairForIdx(unsigned int idx) {
    if (idx >= mCollisions.size()) return COLLISION_PACKED_RESULT_INVALID;
    return mCollisions[idx];
}

bool CollisionWorldOctree::setPositionForPoint(CollisionEntryId id, float x, float y) {
    if (id >= mEntries.size() || mEntries[id].hole) return false;

    mRoot.remove(id, mEntries[id]);
    mEntries[id].x = x;
    mEntries[id].y = y;
    mRoot.insert(id, mEntries, mEntries[id]);
    return true;
}

bool CollisionWorldOctree::checkCollision_(const CollisionEntry& a, const CollisionEntry& b) const {
    //if ((a.mask & b.mask) == 0) return false;

    if (a.shape == CollisionShape::CIRCLE && b.shape == CollisionShape::CIRCLE) {
        return checkCircleCollision(a.x, a.y, a.c.radius, b.x, b.y, b.c.radius);
    } else if (a.shape == CollisionShape::CIRCLE && b.shape == CollisionShape::RECTANGLE) {
        return checkCircleRectangleCollision(a.x, a.y, a.c.radius, b.x, b.y, b.r.width, b.r.height);
    } else if (a.shape == CollisionShape::RECTANGLE && b.shape == CollisionShape::CIRCLE) {
        return checkCircleRectangleCollision(b.x, b.y, b.c.radius, a.x, a.y, a.r.width, a.r.height);
    } else if (a.shape == CollisionShape::RECTANGLE && b.shape == CollisionShape::RECTANGLE) {
        return checkRectangleCollision(a.x, a.y, a.r.width, a.r.height, b.x, b.y, b.r.width, b.r.height);
    }

    return false;
}

// CollisionOctree implementation

CollisionWorldOctree::CollisionOctree::CollisionOctree(const AABB& bounds, int maxDepth)
    : mBounds(bounds), mMaxDepth(maxDepth), mDepth(0), mIsLeaf(true) {}

CollisionWorldOctree::CollisionOctree::~CollisionOctree() {}

void CollisionWorldOctree::CollisionOctree::insert(CollisionEntryId id, std::vector<CollisionEntry>& totalEntries, const CollisionEntry& entry) {
    AABB entryBounds = entry.getBounds();

    // If this entry doesn't fit in our bounds, don't insert it
    if (!mBounds.intersects(entryBounds)) {
        return;
    }

    // If we're a leaf and not full, add the object
    if (mIsLeaf) {
        if (mObjectIds.size() < MAX_OBJECTS || mDepth >= mMaxDepth) {
            mObjectIds.push_back(id);
            return;
        }

        // We're full, split and redistribute
        split(totalEntries);
    }

    // Find the quadrant(s) to insert into
    int quadrant = getQuadrant(entryBounds);

    if (quadrant != -1) {
        // Entry fits in a single quadrant
        mChildren[quadrant].insert(id, totalEntries, entry);
    } else {
        // Entry spans multiple quadrants, store it in this node
        mObjectIds.push_back(id);
    }
}

void CollisionWorldOctree::CollisionOctree::remove(CollisionEntryId id, const CollisionEntry& entry) {
    AABB entryBounds = entry.getBounds();

    // If this entry doesn't fit in our bounds, it's not here
    if (!mBounds.intersects(entryBounds)) {
        return;
    }

    // Try to remove from this node's objects
    auto it = std::find(mObjectIds.begin(), mObjectIds.end(), id);
    if (it != mObjectIds.end()) {
        mObjectIds.erase(it);
        return;
    }

    // If we're a leaf, we're done
    if (mIsLeaf) {
        return;
    }

    // Try to remove from children
    int quadrant = getQuadrant(entryBounds);
    if (quadrant != -1) {
        mChildren[quadrant].remove(id, entry);
    } else {
        // Entry spans multiple quadrants, check all children
        for (CollisionOctree& child : mChildren) {
            child.remove(id, entry);
        }
    }
}

void CollisionWorldOctree::CollisionOctree::query(const CollisionEntry& entry, std::vector<CollisionEntry>& totalEntries, std::vector<CollisionEntry*>& results) {
    AABB entryBounds = entry.getBounds();

    // If this entry doesn't intersect our bounds, it can't collide with anything here
    if (!mBounds.intersects(entryBounds)) {
        return;
    }

    // Add all objects in this node
    for (CollisionEntryId id : mObjectIds) {
        results.push_back(&totalEntries[id]);
    }

    // If we're a leaf, we're done
    if (mIsLeaf) {
        return;
    }

    // Query children
    int quadrant = getQuadrant(entryBounds);
    if (quadrant != -1) {
        mChildren[quadrant].query(entry, totalEntries, results);
    } else {
        // Entry spans multiple quadrants, check all children
        for (CollisionOctree& child : mChildren) {
            child.query(entry, totalEntries, results);
        }
    }
}

void CollisionWorldOctree::CollisionOctree::split(std::vector<CollisionEntry>& totalEntries) {
    if (!mIsLeaf) return;

    mIsLeaf = false;
    int newDepth = mDepth + 1;

    // Create four children
    float midX = (mBounds.minX + mBounds.maxX) * 0.5f;
    float midY = (mBounds.minY + mBounds.maxY) * 0.5f;

    mChildren.resize(4);
    // Top-left
    mChildren[0] = CollisionOctree(AABB(mBounds.minX, mBounds.minY, midX, midY), mMaxDepth);
    mChildren[0].mDepth = newDepth;

    // Top-right
    mChildren[1] = CollisionOctree(AABB(midX, mBounds.minY, mBounds.maxX, midY), mMaxDepth);
    mChildren[1].mDepth = newDepth;

    // Bottom-left
    mChildren[2] = CollisionOctree(AABB(mBounds.minX, midY, midX, mBounds.maxY), mMaxDepth);
    mChildren[2].mDepth = newDepth;

    // Bottom-right
    mChildren[3] = CollisionOctree(AABB(midX, midY, mBounds.maxX, mBounds.maxY), mMaxDepth);
    mChildren[3].mDepth = newDepth;

    // Redistribute objects
    std::vector<CollisionEntryId> objectsToRedistribute = mObjectIds;
    mObjectIds.clear();

    for (CollisionEntryId id : objectsToRedistribute) {
        insert(id, totalEntries, totalEntries[id]);
    }
}

int CollisionWorldOctree::CollisionOctree::getQuadrant(const AABB& bounds) const {
    float midX = (mBounds.minX + mBounds.maxX) * 0.5f;
    float midY = (mBounds.minY + mBounds.maxY) * 0.5f;

    // Check if the bounds fit entirely within one quadrant
    if (bounds.maxX <= midX) {
        if (bounds.maxY <= midY) {
            return 0; // Top-left
        } else if (bounds.minY >= midY) {
            return 2; // Bottom-left
        }
    } else if (bounds.minX >= midX) {
        if (bounds.maxY <= midY) {
            return 1; // Top-right
        } else if (bounds.minY >= midY) {
            return 3; // Bottom-right
        }
    }

    // Bounds span multiple quadrants
    return -1;
}

} // namespace AV
