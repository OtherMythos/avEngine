#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptGetterUtils{
    public:
        ScriptGetterUtils() = delete;

        /**
        Read in a value of three floats from the stack.
        This function can parse either one SlotPosition, one Vec3 or three floats.
        If a SlotPosition is passed it will be converted to a vector3.

        This function assumes that the appropriate values are at the top of the stack.

        Expects a param check of -2, ".n|unn".

        @param outVec
        A pointer to the vector in which the output should be written.

        @returns
        True or false depending on whether the get was successful or not.
        */
        static bool vector3Read(HSQUIRRELVM vm, Ogre::Vector3* outVec);

        /**
        Read a vector3 from either a SlotPosition or a Vector3.
        This function does not assume any param checks, and will attempt to read the value from the index specified.

        @returns
        True or false depending on whether the get was successful or not.
        */
        static bool vector3ReadSlotOrVec(HSQUIRRELVM vm, Ogre::Vector3* outVec, SQInteger idx);

        /**
        Read either three floats or a vector 3 from the stack.

        @param outVec
        A pointer to the vector in which the output should be written.

        @returns
        True or false depending on whether the get was successful or not.
        */
        static UserDataGetResult read3FloatsOrVec3(HSQUIRRELVM vm, Ogre::Vector3* outVec);
    };
}
