#pragma once

#include "System/EnginePrerequisites.h"
#include "PhysicsWorld.h"

#include "Scripting/ScriptDataPacker.h"

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

    private:
        CollisionWorldId mWorldId;

        CollisionWorldThreadLogic* mThreadLogic = 0;

        std::shared_ptr<CollisionWorldDataManager> mDataManager;

        static ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* mCollisionObjectData;

        struct CollisionEventEntry{
            //In future this could just be changed to ints representing the values.
            void* sender;
            void* receiver;
        };
        //A thread side list of the collision events.
        std::vector<CollisionEventEntry> mCollisionEvents;
    };
}
