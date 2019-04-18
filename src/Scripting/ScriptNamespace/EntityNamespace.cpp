#include "EntityNamespace.h"

#include "Logger/Log.h"
#include "Classes/SlotPositionClass.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Tracker/EntityTracker.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"


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
    
    SQInteger EntityNamespace::destroyEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -1);
            
            world->getEntityManager()->destroyEntity(entityId);
            
            //Set the id of this entity to be invalid.
            SQUserPointer p = 0;
            sq_getinstanceup(vm, -1, &p, 0);
            *((eId*)p) = eId::INVALID;
        }
        return 0;
    }

    SQInteger EntityNamespace::trackEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -1);

            world->getEntityManager()->getEntityTracker()->trackEntity(entityId);
        }
        return 0;
    }

    SQInteger EntityNamespace::untrackEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = ScriptUtils::getEID(vm, -1);

            world->getEntityManager()->getEntityTracker()->untrackEntity(entityId);
        }
        return 0;
    }

    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createEntity, "create", 2, ".x");
        _addFunction(vm, createEntityTracked, "createTracked", 2, ".x");
        _addFunction(vm, destroyEntity, "destroy", 2, ".x");

        _addFunction(vm, trackEntity, "track", 2, ".x");
        _addFunction(vm, untrackEntity, "untrack", 2, ".x");
    }
}
