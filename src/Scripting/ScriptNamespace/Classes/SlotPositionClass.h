#pragma once

#include <squirrel.h>
#include "World/Slot/SlotPosition.h"

namespace AV{
    /**
    A class to expose the SlotPosition class and its functionality to squirrel.
    */
    class SlotPositionClass{
    public:
        SlotPositionClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

        /**
        Obtain a SlotPosition from the stack.
        This function expects the details of the Slot Position to be pushed to the stack similar to how they are when a function call happens.
        The function will inspect the size of the stack to determine how many arguments were provided.
        */
        static SlotPosition getSlotFromStack(HSQUIRRELVM vm);
        /**
        Get a c++ SlotPosition from a squirrel SlotPosition instance.

        @param vm
        The vm in which the instance is contained.
        @param instanceIndex
        The index of the instance in the stack.
        @param outSlot
        An output parameter of the obtained SlotPosition.

        @return
        A boolean representing whether or not any errors occured.
        */
        static bool getSlotFromInstance(HSQUIRRELVM vm, SQInteger instanceIndex, SlotPosition* outSlot);

        /**
        Create a new SlotPosition class instance.
        This function will create the instance and push it to the stack.
        */
        static void createNewInstance(HSQUIRRELVM vm, const SlotPosition& pos);

    private:

        static SQInteger slotPositionAdd(HSQUIRRELVM vm);
        static SQInteger slotPositionMinus(HSQUIRRELVM vm);
        static SQInteger SlotPositionGet(HSQUIRRELVM vm);
        static SQInteger slotPositionToString(HSQUIRRELVM vm);
        static SQInteger slotPositionCompare(HSQUIRRELVM vm);
        static SQInteger SlotPositionEquals(HSQUIRRELVM vm);

        static SQInteger toVector3(HSQUIRRELVM vm);
        static SQInteger move(HSQUIRRELVM vm);

        static SQInteger createSlotPosition(HSQUIRRELVM vm);

        static bool _readSlotPositionPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, SlotPosition** outPos);

        static SQObject slotPositionDelegateTableObject;
    };
}
