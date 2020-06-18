#pragma once

#include "System/EnginePrerequisites.h"
#include "PhysicsWorld.h"

#include "Scripting/ScriptDataPacker.h"

namespace AV{
    class CollisionWorldThreadLogic;
    class PhysicsBodyConstructor;

    class CollisionWorld : public PhysicsWorld{
        friend PhysicsBodyConstructor;
    public:
        typedef uint8 CollisionWorldId;

        CollisionWorld(CollisionWorldId id);
        ~CollisionWorld();

        void update();

        void notifyOriginShift(Ogre::Vector3 offset);

        void addObject(PhysicsTypes::CollisionObjectPtr object);
        void removeObject(PhysicsTypes::CollisionObjectPtr object);

        void setCollisionWorldThreadLogic(CollisionWorldThreadLogic* threadLogic);

    private:
        CollisionWorldId mWorldId;

        CollisionWorldThreadLogic* mThreadLogic = 0;

        static ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* mCollisionObjectData;
    };
}
