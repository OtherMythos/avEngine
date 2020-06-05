#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class Vector3UserData{
    public:
        Vector3UserData() = delete;
        ~Vector3UserData() = delete;

        static void setupTable(HSQUIRRELVM vm);

        static void vector3ToUserData(HSQUIRRELVM vm, const Ogre::Vector3& vec);
        static bool readVector3FromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3* outVec);

    private:
        static SQInteger setMetamethod(HSQUIRRELVM vm);
        static SQInteger getMetamethod(HSQUIRRELVM vm);
        static SQInteger vector3ToString(HSQUIRRELVM vm);
        static SQInteger unaryMinusMetamethod(HSQUIRRELVM vm);
        static SQInteger addMetamethod(HSQUIRRELVM vm);
        static SQInteger minusMetamethod(HSQUIRRELVM vm);

        static SQInteger _addMinusMetamethod(HSQUIRRELVM vm, bool addition);

        static SQObject vector3DelegateTableObject;

        static SQInteger createVector3(HSQUIRRELVM vm);

        //Read a user data and return
        static bool _readVector3PtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3** outVec);
    };
}
