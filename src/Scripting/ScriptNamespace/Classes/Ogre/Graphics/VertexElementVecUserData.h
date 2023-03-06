#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class VertexElementVecUserData{
    public:
        VertexElementVecUserData() = delete;
        ~VertexElementVecUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void VertexElementVecObjectToUserData(HSQUIRRELVM vm, Ogre::VertexElement2Vec* vec);

        static UserDataGetResult readVertexElementVecFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::VertexElement2Vec** outVec);

    private:
        static SQObject VertexElementVecDelegateTableObject;

        static SQInteger VertexElementVecToString(HSQUIRRELVM vm);
        static SQInteger VertexElementVecCompare(HSQUIRRELVM vm);
        static SQInteger pushVertexElement(HSQUIRRELVM vm);
        static SQInteger clear(HSQUIRRELVM vm);

        static SQInteger VertexElementVecObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
