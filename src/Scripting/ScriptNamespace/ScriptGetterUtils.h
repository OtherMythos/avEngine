#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptGetterUtils{
    public:
        ScriptGetterUtils() = delete;

        /**
        Read in a value of three floats from the stack.
        This function can parse either one Ogre::Vector3, one Vec3 or three floats.
        If a Ogre::Vector3 is passed it will be converted to a vector3.

        This function assumes that the appropriate values are at the top of the stack.

        Expects a param check of -2, ".n|unn".

        @param outVec
        A pointer to the vector in which the output should be written.

        @returns
        True or false depending on whether the get was successful or not.
        */
        static SQInteger vector3Read(HSQUIRRELVM vm, Ogre::Vector3* outVec);

        /**
        Read a Vec3 from the index specified, without any other param checks.

        @returns
        0 on success, otherwise the result of throwing a script error.
        */
        static SQInteger vector3ReadAt(HSQUIRRELVM vm, Ogre::Vector3* outVec, SQInteger idx);

        /**
        Read either three floats or a vector 3 from the stack.

        @param outVec
        A pointer to the vector in which the output should be written.

        @returns
        True or false depending on whether the get was successful or not.
        */
        static UserDataGetResult read3FloatsOrVec3(HSQUIRRELVM vm, Ogre::Vector3* outVec);

        /**
        Read either two floats or a vector2 from the stack.

        @param outVec
        A pointer to the vector in which the output should be written.
        */
        static UserDataGetResult read2FloatsOrVec2(HSQUIRRELVM vm, Ogre::Vector2* outVec);

        /**
        Read either four floats in rgba order or a ColourValue user data from the stack.

        @param outVec
        A pointer to the vector in which the output should be written.
        */
        static UserDataGetResult read4FloatsOrColourValue(HSQUIRRELVM vm, Ogre::ColourValue* outCol);
    };
}
