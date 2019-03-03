#pragma once

#include <squirrel.h>
#include "World/Slot/SlotPosition.h"

namespace AV{
    class SlotPositionClass{
    public:
        SlotPositionClass() { };

        static void setupClass(HSQUIRRELVM vm);

    private:
        static SlotPosition getSlotFromStack(HSQUIRRELVM vm);
        static void setInstanceFromSlot(HSQUIRRELVM vm, const SlotPosition& pos, int argLength);
        static SlotPosition getSlotFromInstance(HSQUIRRELVM vm, SQInteger instanceIndex);

        static SQInteger slotPositionOperator(HSQUIRRELVM vm, const SlotPosition& result);

        static SQInteger slotPositionConstructor(HSQUIRRELVM vm);
        static SQInteger slotPositionAdd(HSQUIRRELVM vm);
        static SQInteger slotPositionMinus(HSQUIRRELVM vm);

        static SQInteger toVector3(HSQUIRRELVM vm);

        static SQMemberHandle handleX, handleY, handleZ;
        static SQMemberHandle handleSlotX, handleSlotY;
    };
}
