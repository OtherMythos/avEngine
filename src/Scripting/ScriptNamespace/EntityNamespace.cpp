#include "EntityNamespace.h"

#include "Classes/SlotPositionClass.h"
#include "Classes/EntityClass/EntityClass.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Tracker/EntityTracker.h"


namespace AV{

    SQInteger EntityNamespace::createEntity(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            SlotPosition pos;
            if(!SlotPositionClass::getSlotFromInstance(vm, -1, &pos)) return 0;

            eId entity = world->getEntityManager()->createEntity(pos);

            EntityClass::_entityClassFromEID(vm, entity);

        }
        return 1;
    }

    SQInteger EntityNamespace::createEntityTracked(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            SlotPosition pos;
            if(!SlotPositionClass::getSlotFromInstance(vm, -1, &pos)) return 0;

            eId entity = world->getEntityManager()->createEntityTracked(pos);

            EntityClass::_entityClassFromEID(vm, entity);

        }
        return 1;
    }

    SQInteger EntityNamespace::destroyEntity(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &entityId));

            world->getEntityManager()->destroyEntity(entityId);

            EntityClass::invalidateEntityInstance(vm);
        }
        return 0;
    }

    SQInteger EntityNamespace::trackEntity(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &entityId));

            world->getEntityManager()->getEntityTracker()->trackEntity(entityId);
        }
        return 0;
    }

    SQInteger EntityNamespace::untrackEntity(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &entityId));

            world->getEntityManager()->getEntityTracker()->untrackEntity(entityId);
        }
        return 0;
    }

    /**SQNamespace
    @name _entity
    @desc Function to interact with the entity system. Note that many of these functions will fail if the world does not exist.
    */
    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name create
        @param1:slotPosition: The position to create the entity at.
        @desc Create an untracked entity.
        @returns A handle to the created entity.
        */
        ScriptUtils::addFunction(vm, createEntity, "create", 2, ".u");
        /**SQFunction
        @name createTracked
        @param1:slotPosition: The position to create the entity at.
        @desc Create a tracked entity.
        @returns A handle to the created entity.
        */
        ScriptUtils::addFunction(vm, createEntityTracked, "createTracked", 2, ".u");
        /**SQFunction
        @name destroy
        @param1:entityHandle: The entity to destroy.
        @desc Destroy an entity
        */
        ScriptUtils::addFunction(vm, destroyEntity, "destroy", 2, ".x");

        /**SQFunction
        @name destroy
        @param1:entityHandle: The entity to be tracked.
        @desc Set an entity to be tracked.
        */
        ScriptUtils::addFunction(vm, trackEntity, "track", 2, ".x");
        /**SQFunction
        @name untrack
        @param1:entityHandle: The entity to be untracked.
        @desc Set an entity to be untracked.
        */
        ScriptUtils::addFunction(vm, untrackEntity, "untrack", 2, ".x");
    }
}
