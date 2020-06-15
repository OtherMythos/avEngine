#pragma once

#include "System/EnginePrerequisites.h"
#include "PhysicsWorld.h"

namespace AV{
    class CollisionWorld : public PhysicsWorld{
    public:
        typedef uint8 CollisionWorldId;

        CollisionWorld(CollisionWorldId id);
        ~CollisionWorld();

        void update();

        void notifyOriginShift(Ogre::Vector3 offset);

    private:
        CollisionWorldId mWorldId;
    };
}
