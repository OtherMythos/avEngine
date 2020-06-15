#pragma once

#include <memory>
#include "System/EnginePrerequisites.h"

namespace AV{
    class CollisionWorld;
    class DynamicsWorld;
    class Event;

    class PhysicsManager{
    public:
        PhysicsManager();
        ~PhysicsManager();

        void initialise();

        void update();

        bool worldEventReceiver(const Event &e);

        std::shared_ptr<DynamicsWorld> getDynamicsWorld() { return mDynamicsWorld; };

    private:
        std::shared_ptr<DynamicsWorld> mDynamicsWorld;
        std::shared_ptr<CollisionWorld> mCollisionWorlds[MAX_COLLISION_WORLDS];

        int mCreatedCollisionWorlds = 0;
    };
}
