#include "EntityNamespace.h"

#include "Logger/Log.h"
#include "Classes/SlotPositionClass.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"

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

    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createEntity, "create", 2, ".x");
        _addFunction(vm, createEntityTracked, "createTracked", 2, ".x");
    }
}
