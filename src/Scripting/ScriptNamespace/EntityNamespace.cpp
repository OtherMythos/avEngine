#include "EntityNamespace.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "System/BaseSingleton.h"

#include "Classes/Entity/EntityUserData.h"

#include "Entity/EntityManager.h"


namespace AV{

    SQInteger EntityNamespace::createEntity(HSQUIRRELVM vm){

        {
            Ogre::Vector3 pos;
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, -1, &pos));

            eId entity = BaseSingleton::getEntityManager()->createEntity(pos);

            EntityUserData::eIDToUserData(vm, entity);

        }
        return 1;
    }

    SQInteger EntityNamespace::destroyEntity(HSQUIRRELVM vm){

        {
            eId entityId;
            SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &entityId));

            BaseSingleton::getEntityManager()->destroyEntity(entityId);
        }
        return 0;
    }

    /**SQNamespace
    @name _entity
    @desc Functions to interact with the entity system.
    */
    void EntityNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name create
        @param1:Vec3: The position to create the entity at.
        @desc Create an entity.
        @returns A handle to the created entity.
        */
        ScriptUtils::addFunction(vm, createEntity, "create", 2, ".u");
        /**SQFunction
        @name destroy
        @param1:entityHandle: The entity to destroy.
        @desc Destroy an entity
        */
        ScriptUtils::addFunction(vm, destroyEntity, "destroy", 2, ".u");
    }
}
