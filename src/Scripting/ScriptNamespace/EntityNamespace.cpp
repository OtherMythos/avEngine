#include "EntityNamespace.h"

#include "Logger/Log.h"
#include "Classes/SlotPositionClass.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Tracker/EntityTracker.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "World/Entity/Logic/FundamentalLogic.h"

namespace AV{

    SQInteger EntityNamespace::createEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            eId entity = world->getEntityManager()->createEntity(pos);

            _entityClassFromEID(vm, entity);

            return 1;
        }
        return 0;
    }
    
    SQInteger EntityNamespace::createEntityTracked(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);
            
            eId entity = world->getEntityManager()->createEntityTracked(pos);
            
            _entityClassFromEID(vm, entity);
            
            return 1;
        }
        return 0;
    }
    
    SQInteger EntityNamespace::trackEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -2);
            
            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            world->getEntityManager()->getEntityTracker()->trackEntity(entityId, pos);
        }
        return 0;
    }
    
    SQInteger EntityNamespace::untrackEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -2);
            
            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            world->getEntityManager()->getEntityTracker()->untrackEntity(entityId, pos);
        }
        return 0;
    }
        
    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createEntity, "create", 2, ".x");
        _addFunction(vm, createEntityTracked, "createTracked", 2, ".x");
        
        _addFunction(vm, trackEntity, "track", 2, ".x");
        _addFunction(vm, untrackEntity, "untrack", 2, ".x");
    }
}
