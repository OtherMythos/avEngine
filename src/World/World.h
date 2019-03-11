#pragma once

#include "Logger/Log.h"
#include <memory>

namespace Ogre{
    class Camera;
}

namespace AV {
    class WorldSingleton;
    class SlotManager;
    class ChunkRadiusLoader;
    class EntityManager;

    class Event;

    /**
     A class to encapsulate the functionality of the world.

     In itself the world is a collection of a number of pieces.
     This class serves as an encapsulation of all of these things.

     There should only be a single world in the engine at a time.
     This is managed by the WorldSingleton class, and the world shouldn't be created directly.
     */
    class World{
        friend WorldSingleton;
    protected:
        World();
        ~World();

        std::shared_ptr<SlotManager> mSlotManager;
        std::shared_ptr<ChunkRadiusLoader> mChunkRadiusLoader;

        std::shared_ptr<EntityManager> mEntityManager;

    public:
        void update(Ogre::Camera *camera);

        bool worldEventReceiver(const Event &event);

        std::shared_ptr<SlotManager> getSlotManager() { return mSlotManager; };
        std::shared_ptr<EntityManager> getEntityManager() { return mEntityManager; };
    };
}
