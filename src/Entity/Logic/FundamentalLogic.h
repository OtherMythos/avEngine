#pragma once

#include "ComponentLogic.h"

#include "World/Slot/SlotPosition.h"

namespace AV{
    /**
     A class to contain logic for components that all entities possess.
     */
    class FundamentalLogic : public ComponentLogic{
    public:
        static SlotPosition getPosition(eId id);

        static bool getTracked(eId id);
        /**
         Set whether or not the entity is tracked.
         This should only be called by
         */
        static void setTracked(eId id, bool tracked);

        static void serialise(std::ofstream& stream, entityx::Entity& e);

    };
}
