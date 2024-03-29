#ifdef TEST_MODE

#include "TestModeTextureNamespace.h"

#include "System/BaseSingleton.h"
#include "Gui/Rect2d/Rect2dManager.h"

#include "Scripting/ScriptNamespace/Classes/MovableTextureClass.h"

namespace AV{
    SQInteger TestModeTextureNamespace::getNumTextures(HSQUIRRELVM vm){
        int num = BaseSingleton::getRect2dManager()->getNumRects();

        sq_pushinteger(vm, num);

        return 1;
    }

    SQInteger TestModeTextureNamespace::getNumTexturesForLayer(HSQUIRRELVM vm){
        SQInteger layer;
        sq_getinteger(vm, -1, &layer);

        int num = BaseSingleton::getRect2dManager()->getNumRectsForLayer(layer);

        sq_pushinteger(vm, num);

        return 1;
    }

    SQInteger TestModeTextureNamespace::isTextureInLayer(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, -2, &p, 0, false);

        SQInteger layer;
        sq_getinteger(vm, -1, &layer);

        SQBool result = MovableTextureClass::isTextureInLayer((void*)p, layer);

        sq_pushbool(vm, result);

        return 1;
    }

    void TestModeTextureNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["getNumTextures"] = {"", 0, getNumTextures};
        functionMap["getNumTexturesForLayer"] = {".i", 2, getNumTexturesForLayer};
        functionMap["isTextureInLayer"] = {".xi", 3, isTextureInLayer};

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
