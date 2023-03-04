#include "MeshUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "SubMeshUserData.h"

#include "Ogre.h"
#include "OgreMesh2.h"

#include <sstream>

namespace AV{

    SQObject MeshUserData::MeshDelegateTableObject;

    void MeshUserData::MeshToUserData(HSQUIRRELVM vm, Ogre::MeshPtr mesh){
        Ogre::MeshPtr* pointer = (Ogre::MeshPtr*)sq_newuserdata(vm, sizeof(Ogre::MeshPtr));
        memset(pointer, 0, sizeof(Ogre::MeshPtr));
        *pointer = mesh;

        sq_pushobject(vm, MeshDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, MeshTypeTag);
        sq_setreleasehook(vm, -1, MeshObjectReleaseHook);
    }

    UserDataGetResult MeshUserData::readMeshFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MeshPtr* outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != MeshTypeTag){
            outProg->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((Ogre::MeshPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger MeshUserData::MeshObjectReleaseHook(SQUserPointer p, SQInteger size){
        Ogre::MeshPtr* ptr = static_cast<Ogre::MeshPtr*>(p);
        ptr->reset();

        return 0;
    }

    SQInteger MeshUserData::getName(HSQUIRRELVM vm){
        Ogre::MeshPtr mat;
        SCRIPT_ASSERT_RESULT(readMeshFromUserData(vm, 1, &mat));

        const Ogre::String& name = mat->getName();

        sq_pushstring(vm, name.c_str(), name.length());

        return 1;
    }

    SQInteger MeshUserData::getNumSubMeshes(HSQUIRRELVM vm){
        Ogre::MeshPtr mesh;
        SCRIPT_ASSERT_RESULT(readMeshFromUserData(vm, 1, &mesh));

        sq_pushinteger(vm, mesh->getNumSubMeshes());

        return 1;
    }

    SQInteger MeshUserData::getSubMesh(HSQUIRRELVM vm){
        Ogre::MeshPtr mesh;
        SCRIPT_ASSERT_RESULT(readMeshFromUserData(vm, 1, &mesh));

        SQInteger meshIdx;
        sq_getinteger(vm, 2, &meshIdx);

        if(meshIdx < 0) return sq_throwerror(vm, "Submesh idx cannot be negative.");
        uint32 idx = static_cast<uint32>(meshIdx);
        if(idx >= mesh->getNumSubMeshes()){
            return sq_throwerror(vm, "Index out of range.");
        }
        Ogre::SubMesh* subMesh = mesh->getSubMesh(idx);

        SubMeshUserData::SubMeshToUserData(vm, subMesh);

        return 1;
    }


    SQInteger MeshUserData::createSubMesh(HSQUIRRELVM vm){
        Ogre::MeshPtr mesh;
        SCRIPT_ASSERT_RESULT(readMeshFromUserData(vm, 1, &mesh));

        Ogre::SubMesh* subMesh = mesh->createSubMesh();
        SubMeshUserData::SubMeshToUserData(vm, subMesh);

        return 1;
    }

    SQInteger MeshUserData::meshCompare(HSQUIRRELVM vm){
        Ogre::MeshPtr first;
        Ogre::MeshPtr second;

        SCRIPT_ASSERT_RESULT(readMeshFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(readMeshFromUserData(vm, -1, &second));

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    void MeshUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getName, "getName");
        ScriptUtils::addFunction(vm, getNumSubMeshes, "getNumSubMeshes");
        ScriptUtils::addFunction(vm, createSubMesh, "createSubMesh");
        ScriptUtils::addFunction(vm, getSubMesh, "getSubMesh", 2, ".i");
        ScriptUtils::addFunction(vm, meshToString, "_tostring");
        ScriptUtils::addFunction(vm, meshCompare, "_cmp");

        sq_resetobject(&MeshDelegateTableObject);
        sq_getstackobj(vm, -1, &MeshDelegateTableObject);
        sq_addref(vm, &MeshDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger MeshUserData::meshToString(HSQUIRRELVM vm){
        Ogre::MeshPtr mat;
        SCRIPT_ASSERT_RESULT(readMeshFromUserData(vm, 1, &mat));

        std::ostringstream stream;
        stream << "Mesh (\"" << mat->getName() << "\")";
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void MeshUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_PROG_TYPE_VERTEX
        @desc Vertex program.
        */
        //ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_VERTEX", (SQInteger)Ogre::GPT_VERTEX_PROGRAM);
    }
}
