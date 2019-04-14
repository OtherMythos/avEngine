#pragma once

#include <squirrel.h>
#include "World/Slot/SlotPosition.h"

namespace AV{
    /**
    A class to expose the SlotPosition class and its functionality to squirrel.
    */
    class SlotPositionClass{
    public:
        SlotPositionClass() { };

        static void setupClass(HSQUIRRELVM vm);

        /**
        Obtain a SlotPosition from the stack.
        This function expects the details of the Slot Position to be pushed to the stack similar to how they are when a function call happens.
        The function will inspect the size of the stack to determine how many arguments were provided.
        */
        static SlotPosition getSlotFromStack(HSQUIRRELVM vm);
        /**
        Sets the values of a SlotPosition squirrel instance from a c++ squirrel instance.

        @param vm
        The vm in which the instance is contained.
        @param pos
        The c++ slot position to set from
        @param argLength
        How the slot position should be set.
        provide 3 if you want just the slot coordinates set, or 6 if you want the slot coordinates and position set.
        If any other number is provided nothing will be set.
        It is named arg length because some functions provide only two arguments, for instance to set just the slot coordinates.
        @param slotIndex
        The slot index of the target instance in the stack.
        */
        static void setInstanceFromSlot(HSQUIRRELVM vm, const SlotPosition& pos, int argLength, int slotIndex);
        /**
        Get a c++ SlotPosition from a squirrel SlotPosition instance.

        @param vm
        The vm in which the instance is contained.
        @param instanceIndex
        The index of the instance in the stack.

        @return
        A c++ SlotPosition retreived from the squirrel instance.
        */
        static SlotPosition getSlotFromInstance(HSQUIRRELVM vm, SQInteger instanceIndex);

        /**
        Create a new SlotPosition class instance.
        This function will create the instance and push it to the stack.
        */
        static void createNewInstance(HSQUIRRELVM vm);

        /**
        Creates a slot position instance equal to the provided value and pushes it to the stack.
        */
        static void instanceFromSlotPosition(HSQUIRRELVM vm, const SlotPosition& pos);

    private:

        static SQInteger slotPositionOperator(HSQUIRRELVM vm, const SlotPosition& result);

        static SQInteger slotPositionConstructor(HSQUIRRELVM vm);
        static SQInteger slotPositionAdd(HSQUIRRELVM vm);
        static SQInteger slotPositionMinus(HSQUIRRELVM vm);
        static SQInteger slotPositionToString(HSQUIRRELVM vm);
        static SQInteger slotPositionCompare(HSQUIRRELVM vm);

        static SQInteger toVector3(HSQUIRRELVM vm);
        static SQInteger move(HSQUIRRELVM vm);

        static SQMemberHandle handleX, handleY, handleZ;
        static SQMemberHandle handleSlotX, handleSlotY;
    };
}
