#pragma once

#include "Logger/Log.h"
#include <memory>

namespace AV {
    class WorldSingleton;
    class SlotManager;

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

    public:
        void update();

        std::shared_ptr<SlotManager> getSlotManager() { return _slotManager; };
    };
}
