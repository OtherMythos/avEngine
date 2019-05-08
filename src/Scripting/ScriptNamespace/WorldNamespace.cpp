#include "WorldNamespace.h"

#include "World/WorldSingleton.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

#include "Serialisation/SaveHandle.h"

namespace AV{
    SQInteger WorldNamespace::createWorld(HSQUIRRELVM vm){
        SQBool val;
        if(sq_gettype(vm, -1) == OT_STRING){
            //TODO This is TEMPORARY to test the functionality.
            SaveHandle handle;
            handle.saveName = "testSave";
            val = WorldSingleton::createWorld(handle);
        }else{
            val = WorldSingleton::createWorld();
        }
        
        sq_pushbool(vm, val);

        return 1;
    }

    SQInteger WorldNamespace::destroyWorld(HSQUIRRELVM vm){
        SQBool val = WorldSingleton::destroyWorld();
        sq_pushbool(vm, val);

        return 1;
    }

    SQInteger WorldNamespace::getPlayerLoadRadius(HSQUIRRELVM vm){
        //I'm not going to check for the world's existance. I'm not dealing with pointers.

        SQInteger rad = WorldSingleton::getPlayerLoadRadius();
        sq_pushinteger(vm, rad);

        return 1;
    }

    SQInteger WorldNamespace::setPlayerLoadRadius(HSQUIRRELVM vm){
        //This radius is rad dawg
        SQInteger rad = 0;
        sq_getinteger(vm, -1, &rad);

        WorldSingleton::setPlayerLoadRadius(rad);

        return 0;
    }

    SQInteger WorldNamespace::setPlayerPosition(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);
        SlotPosition pos;

        if(nargs == 2){
            pos = SlotPositionClass::getSlotFromInstance(vm, -1);
        }
        else if(nargs == 6){
            pos = SlotPositionClass::getSlotFromStack(vm);
        }

        WorldSingleton::setPlayerPosition(pos);

        return 0;
    }

    SQInteger WorldNamespace::getPlayerPosition(HSQUIRRELVM vm){
        SlotPosition pos = WorldSingleton::getPlayerPosition();
        SlotPositionClass::instanceFromSlotPosition(vm, pos);

        return 1;
    }
    
    SQInteger WorldNamespace::worldReady(HSQUIRRELVM vm){
        SQBool val = WorldSingleton::worldReady();
        
        sq_pushbool(vm, val);
        
        return 1;
    }

    void WorldNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createWorld, "createWorld");
        _addFunction(vm, destroyWorld, "destroyWorld");

        _addFunction(vm, getPlayerLoadRadius, "getPlayerLoadRadius");
        _addFunction(vm, setPlayerLoadRadius, "setPlayerLoadRadius", 2, ".i");

        _addFunction(vm, setPlayerPosition, "setPlayerPosition", -2, ".x|nnnnn");
        _addFunction(vm, getPlayerPosition, "getPlayerPosition", 0, ".");
        
        _addFunction(vm, worldReady, "ready");
    }
}
