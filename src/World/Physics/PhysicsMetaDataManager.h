#pragma once

#include "System/Util/DataPool.h"
#include "World/Entity/eId.h"

namespace AV{
    typedef unsigned int CollisionInternalId;

    class PhysicsMetaDataManager{
    public:
        PhysicsMetaDataManager() = delete;
        ~PhysicsMetaDataManager() = delete;

        static void setup();
        static void shutdown();

        static int createDataForObject();
        static void releaseDataForObject(int e);

        static void setEntityForObject(int id, eId e);

        struct PhysicsObjectMeta{
            CollisionInternalId id;
            eId attachedEntity;
        };

        static PhysicsObjectMeta getObjectMeta(int id);

    private:
        static DataPool<PhysicsObjectMeta> mDataStore;

        static CollisionInternalId _getCollisionObjectInternalId();
        static void _releaseCollisionObjectInternalId(CollisionInternalId id);
    };
}
