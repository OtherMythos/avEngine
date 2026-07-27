#pragma once

#include <mutex>
#include <memory>
#include <set>

#include "Physics/PhysicsTypes.h"
#include "btBulletDynamicsCommon.h"
#include "OgreVector3.h"

namespace AV{
    
    /**
    Base class for physics worlds.
    Contains logic for thread safe communication and interaction with the world.
    */
    class PhysicsWorld{
    public:
        PhysicsWorld();
        ~PhysicsWorld();

    protected:
        bool mShiftPerformedLastFrame = false;
    };
}
