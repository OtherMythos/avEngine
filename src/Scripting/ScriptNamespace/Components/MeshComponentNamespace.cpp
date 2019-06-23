#include "MeshComponentNamespace.h"

#include "Logger/Log.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger MeshComponentNamespace::add(HSQUIRRELVM v){
        SQObjectType objectType = sq_gettype(v, -1);

        if(objectType == OT_STRING){
            const SQChar *meshName;
            sq_getstring(v, -1, &meshName);

            OgreMeshComponentLogic::add(EntityClass::getEID(v, -2), Ogre::String(meshName));
        }else if(objectType == OT_INSTANCE){
            OgreMeshManager::OgreMeshPtr mesh = MeshClass::instanceToMeshPtr(v, -1);

            OgreMeshComponentLogic::add(EntityClass::getEID(v, -2), mesh);
        }else{
            return 0;
        }

        return 0;
    }

    SQInteger MeshComponentNamespace::remove(HSQUIRRELVM v){
        eId id = EntityClass::getEID(v, -1);

        OgreMeshComponentLogic::remove(id);

        return 0;
    }

    SQInteger MeshComponentNamespace::getMesh(HSQUIRRELVM v){
        eId id = EntityClass::getEID(v, -1);

        OgreMeshManager::OgreMeshPtr mesh = OgreMeshComponentLogic::getMesh(id);
        if(!mesh){
            return 0;
        }

        MeshClass::MeshPtrToInstance(v, mesh);

        return 1;
    }

    void MeshComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("mesh"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".xs|x");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");
        ScriptUtils::addFunction(vm, getMesh, "getMesh", 0, "");

        sq_newslot(vm, -3, false);
    }
}
