#include "TerrainObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "World/Slot/Chunk/Terrain/TerrainObject.h"

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

    SQInteger TerrainObjectUserData::getCentre(HSQUIRRELVM vm){
        TerrainObject* t;
        SCRIPT_CHECK_RESULT(readTerrainObjectFromUserData(vm, -1, &t));

        return 1;
    }

    SQInteger TerrainObjectUserData::update(HSQUIRRELVM vm){
        TerrainObject* t;
        SCRIPT_CHECK_RESULT(readTerrainObjectFromUserData(vm, -1, &t));

        t->update();

        return 1;
    }

    void TerrainObjectUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getCentre, "getCentre");
        ScriptUtils::addFunction(vm, update, "update");

        sq_resetobject(&terrainObjectDelegateTableObject);
        sq_getstackobj(vm, -1, &terrainObjectDelegateTableObject);
        sq_addref(vm, &terrainObjectDelegateTableObject);
        sq_pop(vm, 1);
    }
}
