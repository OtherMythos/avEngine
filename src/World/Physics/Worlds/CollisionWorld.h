#pragma once

#include "System/EnginePrerequisites.h"
#include "PhysicsWorld.h"

#include "Scripting/ScriptDataPacker.h"
#include "CollisionWorldUtils.h"

namespace AV{
    class CollisionWorldThreadLogic;
    class PhysicsBodyConstructor;
    class CollisionWorldDataManager;

    class CollisionWorld : public PhysicsWorld{
        friend PhysicsBodyConstructor;
    public:
        typedef uint8 CollisionWorldId;

        CollisionWorld(CollisionWorldId id, std::shared_ptr<CollisionWorldDataManager> dataManager);
        ~CollisionWorld();

        void update();

        void notifyOriginShift(Ogre::Vector3 offset);

        void addObject(PhysicsTypes::CollisionObjectPtr object);
        void removeObject(PhysicsTypes::CollisionObjectPtr object);

        void setCollisionWorldThreadLogic(CollisionWorldThreadLogic* threadLogic);

        void setObjectPosition(PhysicsTypes::CollisionObjectPtr object, const btVector3& pos);

        //Should only be called at shared pointer destruction.
        static void _removeObject(const btCollisionObject* object);

    private:
        CollisionWorldId mWorldId;

        CollisionWorldThreadLogic* mThreadLogic = 0;

        std::shared_ptr<CollisionWorldDataManager> mDataManager;

        static ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* mCollisionObjectData;

        struct CollisionEventEntry{
            //In future this could just be changed to ints representing the values.
            const btCollisionObject* sender;
            const btCollisionObject* receiver;
            CollisionObjectEventMask::CollisionObjectEventMask eventMask;
        };
        //A thread side list of the collision events.
        std::vector<CollisionEventEntry> mCollisionEvents;
        std::set<btCollisionObject*> mObjectsInWorld;

        /**
        Clear a value from the thread logic's input buffer.
        This function should only be called when the mutex is held for the collision world.
        */
        void _resetBufferEntries(btCollisionObject* o);
        inline bool _objectInWorld(btCollisionObject* bdy) const;
    };
}
