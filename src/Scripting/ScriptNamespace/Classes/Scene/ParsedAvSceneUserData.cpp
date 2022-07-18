#include "ParsedAvSceneUserData.h"

#include "World/WorldSingleton.h"
#include "World/Nav/NavMeshManager.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    //SQObject ParsedAvSceneUserData::sceneObjectDelegateTable;

    void ParsedAvSceneUserData::sceneObjectToUserData(HSQUIRRELVM vm, ParsedSceneFile* sceneFile){
        ParsedSceneFile** pointer = (ParsedSceneFile**)sq_newuserdata(vm, sizeof(ParsedSceneFile*));
        *pointer = sceneFile;

        //No delegate table needed yet.
        //sq_pushobject(vm, sceneObjectDelegateTable);
        //sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, AvSceneObjectTypeTag);
        sq_setreleasehook(vm, -1, SceneObjectReleaseHook);
    }

    UserDataGetResult ParsedAvSceneUserData::readSceneObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, ParsedSceneFile** outScene){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AvSceneObjectTypeTag){
            *outScene = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        ParsedSceneFile** p = static_cast<ParsedSceneFile**>(pointer);
        *outScene = *p;

        return USER_DATA_GET_SUCCESS;
    }

    void ParsedAvSceneUserData::setupDelegateTable(HSQUIRRELVM vm){
        /*sq_newtableex(vm, 2);

        sq_settypetag(vm, -1, AvSceneObjectTypeTag);
        sq_resetobject(&sceneObjectDelegateTable);
        sq_getstackobj(vm, -1, &sceneObjectDelegateTable);
        sq_addref(vm, &sceneObjectDelegateTable);
        sq_pop(vm, 1);*/
    }

    SQInteger ParsedAvSceneUserData::SceneObjectReleaseHook(SQUserPointer p, SQInteger size){
        ParsedSceneFile** ptr = static_cast<ParsedSceneFile**>(p);
        delete *ptr;

        return 0;
    }
}
