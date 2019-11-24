#include "TestModeTextureNamespace.h"

#include "System/BaseSingleton.h"
#include "Gui/Texture2d/MovableTextureManager.h"

namespace AV{
    SQInteger TestModeTextureNamespace::getNumTextures(HSQUIRRELVM vm){
        int num = BaseSingleton::getMovableTextureManager()->getNumTextures();

        sq_pushinteger(vm, num);

        return 1;
    }

    void TestModeTextureNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        RedirectFunctionMap functionMap;
        functionMap["getNumTextures"] = {"", 0, getNumTextures};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
