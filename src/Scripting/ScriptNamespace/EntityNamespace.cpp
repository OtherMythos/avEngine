#include "EntityNamespace.h"

#include "Logger/Log.h"
#include "Classes/SlotPositionClass.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"

namespace AV{

    SQInteger EntityNamespace::setEntityPosition(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){

            eId entityId = _getEID(vm, -2);

            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            world->getEntityManager()->setEntityPosition(entityId, pos);

            AV_INFO(pos);
        }
        return 0;
    }

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

    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, setEntityPosition, "setPosition", 3, ".xx");
        _addFunction(vm, createEntity, "create", 2, ".x");

        // MeshComponentNamespace meshComponentNamespace;
        // meshComponentNamespace.setupNamespace(vm);
    }
}
