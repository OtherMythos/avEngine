#pragma once

#include <memory>
#include "System/EnginePrerequisites.h"

namespace AV{
    class CollisionWorld;
    class DynamicsWorld;
    class CollisionWorld;
    class Event;

    class PhysicsManager{
    public:
        PhysicsManager();
        ~PhysicsManager();

        void initialise();

        void update();

        bool worldEventReceiver(const Event &e);

        std::shared_ptr<DynamicsWorld> getDynamicsWorld() { return mDynamicsWorld; };

        /**
        Obtain an instance of the collision world from an id.

        @param worldId.
        The id of a collsion world. This should be between 0 and MAX_COLLISION_WORLDS.

        @returns
        A pointer to a collision world by id.
        If the id is invalid, a null pointer will be returned.
        */
        CollisionWorld* getCollisionWorld(uint8 worldId);

    private:
        std::shared_ptr<DynamicsWorld> mDynamicsWorld;
        std::shared_ptr<CollisionWorld> mCollisionWorlds[MAX_COLLISION_WORLDS];

        int mCreatedCollisionWorlds = 0;
    };
}
