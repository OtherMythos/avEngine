#include "TerrainObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "World/Slot/Chunk/Terrain/TerrainObject.h"
#include "World/Slot/Chunk/Terrain/terra/Terra.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

namespace AV{
    SQObject TerrainObjectUserData::terrainObjectDelegateTableObject;

    void TerrainObjectUserData::TerrainObjectToUserData(HSQUIRRELVM vm, TerrainObject* object){
        TerrainObject** pointer = (TerrainObject**)sq_newuserdata(vm, sizeof(TerrainObject*));
        *pointer = object;

        sq_pushobject(vm, terrainObjectDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, TerrainTypeTag);
    }

    UserDataGetResult TerrainObjectUserData::readTerrainObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TerrainObject** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != TerrainTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        TerrainObject** p = (TerrainObject**)pointer;
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger TerrainObjectUserData::setRenderQueueGroup(HSQUIRRELVM vm){
        TerrainObject* t;
        SCRIPT_CHECK_RESULT(readTerrainObjectFromUserData(vm, 1, &t));

        SQInteger queueId;
        sq_getinteger(vm, 2, &queueId);

        t->getTerra()->setRenderQueueGroup(queueId);

        return 1;
    }

    SQInteger TerrainObjectUserData::update(HSQUIRRELVM vm){
        TerrainObject* t;
        SCRIPT_CHECK_RESULT(readTerrainObjectFromUserData(vm, -1, &t));

        t->update();

        return 1;
    }

    SQInteger TerrainObjectUserData::load(HSQUIRRELVM vm){
        TerrainObject* t;
        SCRIPT_CHECK_RESULT(readTerrainObjectFromUserData(vm, 1, &t));

        const SQChar *textureName;
        sq_getstring(vm, 2, &textureName);

        Ogre::Vector3 origin;
        Ogre::Vector3 size;
        Vector3UserData::readVector3FromUserData(vm, 3, &origin);
        Vector3UserData::readVector3FromUserData(vm, 4, &size);

        t->load(textureName, origin, size);

        return 1;
    }

    void TerrainObjectUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, load, "load", 4, ".suu");
        ScriptUtils::addFunction(vm, setRenderQueueGroup, "setRenderQueueGroup", 2, ".i");
        ScriptUtils::addFunction(vm, update, "update");

        sq_resetobject(&terrainObjectDelegateTableObject);
        sq_getstackobj(vm, -1, &terrainObjectDelegateTableObject);
        sq_addref(vm, &terrainObjectDelegateTableObject);
        sq_pop(vm, 1);
    }
}
