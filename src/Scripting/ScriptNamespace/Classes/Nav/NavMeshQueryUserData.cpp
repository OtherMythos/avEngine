#include "NavMeshQueryUserData.h"

#include "World/WorldSingleton.h"
#include "World/Nav/NavMeshManager.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

namespace AV{

    SQObject NavMeshQueryUserData::queryDelegateTable;

    SQInteger NavMeshQueryUserData::navMeshQueryReleaseHook(SQUserPointer p, SQInteger size){
        NavQueryId* ptr = static_cast<NavQueryId*>(p);

        World* w = WorldSingleton::getWorldNoCheck();
        //The objects will already have been destroyed if the world doesn't exist.
        //TODO what if the world is re-created but the id persists? Is this a problem?
        if(!w) return 0;
        w->getNavMeshManager()->releaseNavMeshQuery(*ptr);

        return 0;
    }

    void NavMeshQueryUserData::queryToUserData(HSQUIRRELVM vm, NavQueryId query){
        NavQueryId* pointer = (NavQueryId*)sq_newuserdata(vm, sizeof(NavQueryId));
        *pointer = query;

        sq_pushobject(vm, queryDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, NavMeshQueryTypeTag);
        sq_setreleasehook(vm, -1, navMeshQueryReleaseHook);
    }

    UserDataGetResult NavMeshQueryUserData::readQueryFromUserData(HSQUIRRELVM vm, SQInteger stackInx, NavQueryId* outQuery){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != NavMeshQueryTypeTag){
            *outQuery = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        NavQueryId* p = static_cast<NavQueryId*>(pointer);
        *outQuery = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger NavMeshQueryUserData::isQueryValid(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        NavQueryId outId;
        SCRIPT_ASSERT_RESULT(readQueryFromUserData(vm, 1, &outId));
        bool result = world->getNavMeshManager()->isNavMeshQueryValid(outId);

        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger NavMeshQueryUserData::findNearestPoly(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        NavQueryId outId;
        SCRIPT_ASSERT_RESULT(readQueryFromUserData(vm, 1, &outId));
        dtNavMeshQuery* query = world->getNavMeshManager()->getQuery(outId);

        Ogre::Vector3 startOgre;
        Ogre::Vector3 extendOgre;

        SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 2, &startOgre));
        SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &extendOgre));

        float startPos[3] = {startOgre.x, startOgre.y, startOgre.z};
        float extend[3] = {extendOgre.x, extendOgre.y, extendOgre.z};
        dtQueryFilter filter;
        dtPolyRef startRef;
        float startPolyPoint[3];

        dtStatus result = query->findNearestPoly(startPos, extend, &filter, &startRef, &(startPolyPoint[0]));

        SQInteger retVal = startRef;
        if(result != DT_SUCCESS) retVal = -1;
        sq_pushinteger(vm, retVal);

        return 1;
    }

    SQInteger NavMeshQueryUserData::moveAlongSurface(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        NavQueryId outId;
        SCRIPT_ASSERT_RESULT(readQueryFromUserData(vm, 1, &outId));
        dtNavMeshQuery* query = world->getNavMeshManager()->getQuery(outId);

        SQInteger startPoly;
        Ogre::Vector3 startOgre;
        Ogre::Vector3 endOgre;

        sq_getinteger(vm, 2, &startPoly);
        SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &startOgre));
        SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 4, &endOgre));

        float startPos[3] = {startOgre.x, startOgre.y, startOgre.z};
        float endPos[3] = {endOgre.x, endOgre.y, endOgre.z};
        dtQueryFilter filter;
        dtPolyRef startRef;
        float finalPos[3];

        static const int MAX_VISITED = 5;
        dtPolyRef visited[MAX_VISITED];
        int visitedCount;

        dtStatus result = query->moveAlongSurface(startPoly, startPos, endPos, &filter, &(finalPos[0]), &(visited[0]), &visitedCount, MAX_VISITED);

        if(result == DT_SUCCESS){
            Vector3UserData::vector3ToUserData(vm, Ogre::Vector3(finalPos[0], finalPos[1], finalPos[2]));
        }else{
            Vector3UserData::vector3ToUserData(vm, startOgre);
        }

        return 1;
    }

    void NavMeshQueryUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 2);

        ScriptUtils::addFunction(vm, isQueryValid, "isValid");

        ScriptUtils::addFunction(vm, findNearestPoly, "findNearestPoly", 3, ".uu");
        ScriptUtils::addFunction(vm, moveAlongSurface, "moveAlongSurface", 4, ".iuu");

        sq_settypetag(vm, -1, NavMeshQueryTypeTag);
        sq_resetobject(&queryDelegateTable);
        sq_getstackobj(vm, -1, &queryDelegateTable);
        sq_addref(vm, &queryDelegateTable);
        sq_pop(vm, 1);
    }
}
