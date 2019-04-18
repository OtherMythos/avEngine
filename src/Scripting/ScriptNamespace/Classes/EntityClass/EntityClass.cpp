#include "EntityClass.h"

#include "World/WorldSingleton.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "World/Entity/EntityManager.h"

#include "World/Slot/ChunkRadiusLoader.h"
#include "World/Entity/Logic/FundamentalLogic.h"

namespace AV{

    SQInteger EntityClass::setEntityPosition(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -2);

            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityClass::checkValid(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -1);

            SQBool result = world->getEntityManager()->getEntityValid(entityId);
            sq_pushbool(vm, result);

            return 1;
        }
        return 0;
    }

    SQInteger EntityClass::checkTrackable(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -1);

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            bool viableChunk = WorldSingleton::getWorld()->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());

            sq_pushbool(vm, viableChunk);

            return 1;
        }
        return 0;
    }

    SQInteger EntityClass::isTracked(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -1);

            SQBool tracked = FundamentalLogic::getTracked(entityId);

            sq_pushbool(vm, tracked);

            return 1;
        }
        return 0;
    }

    void EntityClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("entity"), -1);
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("setPosition"), -1);
        sq_newclosure(vm, setEntityPosition, 0);
        sq_setparamscheck(vm,2,_SC(".x"));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("valid"), -1);
        sq_newclosure(vm, checkValid, 0);
        sq_setparamscheck(vm,1,_SC("."));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("trackable"), -1);
        sq_newclosure(vm, checkTrackable, 0);
        sq_setparamscheck(vm,1,_SC("."));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("tracked"), -1);
        sq_newclosure(vm, isTracked, 0);
        sq_setparamscheck(vm,1,_SC("."));
        sq_newslot(vm, -3, false);

        sq_newslot(vm, -3 , false);
    }
}
