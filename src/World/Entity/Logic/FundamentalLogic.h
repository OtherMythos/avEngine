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
        
    };
}

