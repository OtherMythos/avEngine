#pragma once

#include <squirrel.h>
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Slot/Recipe/AvScene/AvSceneParsedData.h"

namespace AV{
    class ParsedAvSceneUserData{
    public:
        ParsedAvSceneUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void sceneObjectToUserData(HSQUIRRELVM vm, ParsedSceneFile* mesh);
        static UserDataGetResult readSceneObjectFromUserData(HSQUIRRELVM vm, SQInteger stackIndex, ParsedSceneFile** outMeshId);

    private:
        // static SQInteger testRay(HSQUIRRELVM vm);
        static SQInteger SceneObjectReleaseHook(SQUserPointer p, SQInteger size);

        //static SQObject sceneObjectDelegateTable;
    };
}
