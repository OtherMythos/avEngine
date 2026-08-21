#pragma once

#include <squirrel.h>
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Slot/Recipe/AvScene/AvSceneParsedData.h"

namespace AV{
    /**
    What a parsed scene node userdata holds. Nodes are just indices into the parsed scene's
    data, so the shared pointer is what keeps that index meaningful. Without it a script could
    drop the last reference to the scene and carry on reading nodes out of freed memory.
    */
    struct ParsedSceneNodeData{
        ParsedSceneFilePtr scene;
        uint32 index;
    };

    class ParsedAvSceneNodeUserData{
    public:
        ParsedAvSceneNodeUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void sceneNodeToUserData(HSQUIRRELVM vm, ParsedSceneFilePtr scene, uint32 index);
        static UserDataGetResult readSceneNodeFromUserData(HSQUIRRELVM vm, SQInteger stackIndex, ParsedSceneNodeData** outData);

    private:
        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger getTag(HSQUIRRELVM vm);
        static SQInteger getType(HSQUIRRELVM vm);
        static SQInteger getMeshName(HSQUIRRELVM vm);
        static SQInteger getUserValue(HSQUIRRELVM vm);
        static SQInteger getAnimIdx(HSQUIRRELVM vm);

        static SQInteger getPosition(HSQUIRRELVM vm);
        static SQInteger getScale(HSQUIRRELVM vm);
        static SQInteger getOrientation(HSQUIRRELVM vm);
        static SQInteger getDerivedPosition(HSQUIRRELVM vm);
        static SQInteger getDerivedScale(HSQUIRRELVM vm);
        static SQInteger getDerivedOrientation(HSQUIRRELVM vm);

        static SQInteger getParent(HSQUIRRELVM vm);
        static SQInteger getNumChildren(HSQUIRRELVM vm);
        static SQInteger getChild(HSQUIRRELVM vm);

        static SQInteger nodeToString(HSQUIRRELVM vm);

        static SQInteger nodeReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject sceneNodeDelegateTable;
    };
}
