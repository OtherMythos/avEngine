#pragma once

#include <squirrel.h>
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Slot/Recipe/AvScene/AvSceneParsedData.h"

namespace AV{
    class ParsedAvSceneUserData{
    public:
        ParsedAvSceneUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void sceneObjectToUserData(HSQUIRRELVM vm, ParsedSceneFilePtr sceneFile);
        static UserDataGetResult readSceneObjectFromUserData(HSQUIRRELVM vm, SQInteger stackIndex, ParsedSceneFilePtr* outScene);

    private:
        static SQInteger getNodeByTag(HSQUIRRELVM vm);
        static SQInteger hasTag(HSQUIRRELVM vm);
        static SQInteger getNumRootNodes(HSQUIRRELVM vm);
        static SQInteger getRootNode(HSQUIRRELVM vm);
        static SQInteger sceneToString(HSQUIRRELVM vm);

        static SQInteger SceneObjectReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject sceneObjectDelegateTable;
    };
}
