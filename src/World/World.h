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

    class Event;

    /**
     A class to encapsulate the functionality of the world.

     In itself the world is a collection of a number of pieces.
     This class serves as an encapsulation of all of these things.
     */
    class World{
        friend WorldSingleton;
    protected:
        World();
        ~World();

        std::shared_ptr<SlotManager> _slotManager;
        std::shared_ptr<ChunkRadiusLoader> mChunkRadiusLoader;

    public:
        void update(Ogre::Camera *camera);

        bool testReceiver(const Event &event);

        std::shared_ptr<SlotManager> getSlotManager() { return _slotManager; };
    };
}
