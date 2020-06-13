#include "WorldNamespace.h"

#include "World/WorldSingleton.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptNamespace/Classes/SaveHandleClass.h"

#include "Serialisation/SaveHandle.h"

namespace AV{
    SQInteger WorldNamespace::createWorld(HSQUIRRELVM vm){
        SQBool val;
        if(sq_gettype(vm, -1) == OT_INSTANCE){
            SaveHandle handle = SaveHandleClass::instanceToSaveHandle(vm);

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
            if(!SlotPositionClass::getSlotFromInstance(vm, -1, &pos)) return 0;
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

    SQInteger WorldNamespace::worldCreatedFromSave(HSQUIRRELVM vm){
        World* w = WorldSingleton::getWorldNoCheck();
        if(w){
            sq_pushbool(vm, w->createdFromSave());
        }else{
            sq_pushnull(vm);
        }

        return 1;
    }

    SQInteger WorldNamespace::getWorldCreatorHandle(HSQUIRRELVM vm){
        World* w = WorldSingleton::getWorldNoCheck();
        if(w){
            const SaveHandle& handle = w->getCreatorSaveHandle();

            SaveHandleClass::saveHandleToInstance(vm, handle);
        }else{
            sq_pushnull(vm);
        }

        return 1;
    }

    SQInteger WorldNamespace::serialiseWorld(HSQUIRRELVM vm){
        World* w = WorldSingleton::getWorld();
        if(w){
            SaveHandle handle = SaveHandleClass::instanceToSaveHandle(vm);

            w->serialise(handle);
        }
        return 0;
    }

    /**SQNamespace
    @name _world
    @desc Functions specific to the operation of the world.
    */
    void WorldNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name createWorld
        @desc Create the world. This assumes no world exists previously.
        */
        ScriptUtils::addFunction(vm, createWorld, "createWorld");
        /**SQFunction
        @name destroyWorld
        @desc Destroy the world. This assumes world exists.
        */
        ScriptUtils::addFunction(vm, destroyWorld, "destroyWorld");

        /**SQFunction
        @name getPlayerLoadRadius
        @returns Returns the load radius for the world. This defines how many chunks should be loaded within the player radius.
        */
        ScriptUtils::addFunction(vm, getPlayerLoadRadius, "getPlayerLoadRadius");
        ScriptUtils::addFunction(vm, setPlayerLoadRadius, "setPlayerLoadRadius", 2, ".i");

        ScriptUtils::addFunction(vm, setPlayerPosition, "setPlayerPosition", -2, ".x|nnnnn");
        ScriptUtils::addFunction(vm, getPlayerPosition, "getPlayerPosition", 0, ".");

        ScriptUtils::addFunction(vm, serialiseWorld, "serialise", 2, ".x");

        ScriptUtils::addFunction(vm, worldReady, "ready");
        ScriptUtils::addFunction(vm, worldCreatedFromSave, "createdFromSave");
        ScriptUtils::addFunction(vm, getWorldCreatorHandle, "getCreatorHandle");
    }
}
