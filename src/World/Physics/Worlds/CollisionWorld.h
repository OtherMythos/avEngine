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

        void addSender(PhysicsTypes::CollisionSenderPtr sender);
        void removeSender(PhysicsTypes::CollisionSenderPtr sender);

        void setCollisionWorldThreadLogic(CollisionWorldThreadLogic* threadLogic);

    private:
        CollisionWorldId mWorldId;

        CollisionWorldThreadLogic* mThreadLogic = 0;

        static ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* mCollisionObjectData;
    };
}
