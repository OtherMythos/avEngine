#include "MeshComponentNamespace.h"

#include "Logger/Log.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "Scripting/ScriptNamespace/Classes/Entity/EntityUserData.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger MeshComponentNamespace::add(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);

        eId id;
        if(objectType == OT_STRING){
            const SQChar *meshName;
            sq_getstring(vm, -1, &meshName);

            SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -2, &id));
            OgreMeshComponentLogic::add(id, Ogre::String(meshName));
        }else if(objectType == OT_INSTANCE){
            OgreMeshManager::OgreMeshPtr mesh = MeshClass::instanceToMeshPtr(vm, -1);

            SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -2, &id));
            OgreMeshComponentLogic::add(id, mesh);
        }else{
            return 0;
        }

        return 0;
    }

    SQInteger MeshComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        OgreMeshComponentLogic::remove(id);

        return 0;
    }

    SQInteger MeshComponentNamespace::getMesh(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        OgreMeshManager::OgreMeshPtr mesh = OgreMeshComponentLogic::getMesh(id);
        if(!mesh){
            return 0;
        }

        MeshClass::MeshPtrToInstance(vm, mesh);

        return 1;
    }

    void MeshComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("mesh"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".us|x");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".u");
        ScriptUtils::addFunction(vm, getMesh, "getMesh", 2, ".u");

        sq_newslot(vm, -3, false);
    }
}
