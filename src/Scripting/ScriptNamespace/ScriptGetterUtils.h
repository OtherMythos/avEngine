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

        Expects a param check of -2, ".n|u|xnn".

        @param outVec
        A pointer to the vector in which the output should be written.

        @returns
        True or false depending on whether the get was successful or not.
        */
        static bool vector3Read(HSQUIRRELVM vm, Ogre::Vector3* outVec);
    };
}
