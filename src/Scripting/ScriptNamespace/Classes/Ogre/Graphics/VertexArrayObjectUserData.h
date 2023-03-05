#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class VertexArrayObjectUserData{
    public:
        VertexArrayObjectUserData() = delete;
        ~VertexArrayObjectUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void VertexArrayObjectToUserData(HSQUIRRELVM vm, Ogre::VertexArrayObject* program);

        static UserDataGetResult readVertexArrayObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::VertexArrayObject** outProg);

    private:
        static SQObject VertexArrayObjectDelegateTableObject;

        static SQInteger getMaterialName(HSQUIRRELVM vm);
        static SQInteger VertexArrayObjectToString(HSQUIRRELVM vm);
        static SQInteger VertexArrayObjectCompare(HSQUIRRELVM vm);

        static SQInteger VertexArrayObjectObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
