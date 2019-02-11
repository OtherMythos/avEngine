#include "WorldNamespace.h"

#include "World/WorldSingleton.h"

namespace AV{
    SQInteger WorldNamespace::createWorld(HSQUIRRELVM vm){
        SQBool val = WorldSingleton::createWorld();
        sq_pushbool(vm, val);
        
        return 1;
    }
    
    SQInteger WorldNamespace::destroyWorld(HSQUIRRELVM vm){
        SQBool val = WorldSingleton::destroyWorld();
        sq_pushbool(vm, val);
        
        return 1;
    }

    void WorldNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createWorld, "createWorld");
        _addFunction(vm, destroyWorld, "destroyWorld");
    }
}
