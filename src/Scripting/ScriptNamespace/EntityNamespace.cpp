#include "EntityNamespace.h"

#include "Logger/Log.h"
#include "Classes/SlotPositionClass.h"

#include "Components/MeshComponentNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"

namespace AV{

    SQInteger EntityNamespace::setEntityPosition(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){

            eId entityId = _getEID(vm, -2);

            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityNamespace::createEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            eId entity = world->getEntityManager()->createEntity(pos);

            _wrapEID(vm, entity);
            return 1;
        }
        return 0;
    }

    void EntityNamespace::_wrapEID(HSQUIRRELVM vm, eId entity){
        squirrelEIdData** ud = reinterpret_cast<squirrelEIdData**>(sq_newuserdata(vm, sizeof (squirrelEIdData*)));
        *ud = new squirrelEIdData(entity);

        //TODO add the release hook here.
    }

    eId EntityNamespace::_getEID(HSQUIRRELVM vm, int stackIndex){
        SQUserPointer p;
        sq_getuserdata(vm, stackIndex, &p, NULL);
        squirrelEIdData **data = static_cast<squirrelEIdData**>(p);

        squirrelEIdData* pointer = *data;

        return pointer->id;
    }

    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, setEntityPosition, "setPosition", 3, ".ux");
        _addFunction(vm, createEntity, "create", 2, ".x");

        MeshComponentNamespace meshComponentNamespace;
        meshComponentNamespace.setupNamespace(vm);
    }
}
