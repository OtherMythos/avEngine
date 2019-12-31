#include "WindowNamespace.h"

#include "System/BaseSingleton.h"
#include "Window/Window.h"

namespace AV{

    SQInteger WindowNamespace::getWidth(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getWindow()->getWidth());

        return 1;
    }

    SQInteger WindowNamespace::getHeight(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getWindow()->getHeight());

        return 1;
    }

    void WindowNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getWidth, "getWidth");
        ScriptUtils::addFunction(vm, getHeight, "getHeight");
    }
}
