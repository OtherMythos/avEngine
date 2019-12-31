#include "EntityNamespace.h"

#include "Classes/SlotPositionClass.h"
#include "Classes/EntityClass/EntityClass.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Tracker/EntityTracker.h"


namespace AV{

    SQInteger EntityNamespace::createEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            eId entity = world->getEntityManager()->createEntity(pos);

            EntityClass::_entityClassFromEID(vm, entity);

            return 1;
        }
        return 0;
    }

    SQInteger EntityNamespace::createEntityTracked(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            eId entity = world->getEntityManager()->createEntityTracked(pos);

            EntityClass::_entityClassFromEID(vm, entity);

            return 1;
        }
        return 0;
    }

    SQInteger EntityNamespace::destroyEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            //TODO SORT THIS OUT!
            eId entityId = EntityClass::getEID(vm, -1);

            world->getEntityManager()->destroyEntity(entityId);

            EntityClass::invalidateEntityInstance(vm);
        }
        return 0;
    }

    SQInteger EntityNamespace::trackEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = EntityClass::getEID(vm, -1);

            world->getEntityManager()->getEntityTracker()->trackEntity(entityId);
        }
        return 0;
    }

    SQInteger EntityNamespace::untrackEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = EntityClass::getEID(vm, -1);

            world->getEntityManager()->getEntityTracker()->untrackEntity(entityId);
        }
        return 0;
    }

    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, createEntity, "create", 2, ".x");
        ScriptUtils::addFunction(vm, createEntityTracked, "createTracked", 2, ".x");
        ScriptUtils::addFunction(vm, destroyEntity, "destroy", 2, ".x");

        ScriptUtils::addFunction(vm, trackEntity, "track", 2, ".x");
        ScriptUtils::addFunction(vm, untrackEntity, "untrack", 2, ".x");
    }
}
