#include "SubMeshUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Ogre.h"
#include "OgreSubMesh2.h"

#include <sstream>

namespace AV{

    SQObject SubMeshUserData::SubMeshDelegateTableObject;

    void SubMeshUserData::SubMeshToUserData(HSQUIRRELVM vm, Ogre::SubMesh* subMesh){
        Ogre::SubMesh** pointer = (Ogre::SubMesh**)sq_newuserdata(vm, sizeof(Ogre::SubMesh*));
        *pointer = subMesh;

        sq_pushobject(vm, SubMeshDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, SubMeshTypeTag);
    }

    UserDataGetResult SubMeshUserData::readSubMeshFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SubMesh** outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != SubMeshTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = (Ogre::SubMesh*)pointer;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger SubMeshUserData::getMaterialName(HSQUIRRELVM vm){
        Ogre::SubMesh* mesh;
        SCRIPT_ASSERT_RESULT(readSubMeshFromUserData(vm, 1, &mesh));

        Ogre::String matName = mesh->getMaterialName();

        sq_pushstring(vm, matName.c_str(), matName.length());

        return 1;
    }

    SQInteger SubMeshUserData::SubMeshCompare(HSQUIRRELVM vm){
        Ogre::SubMesh* first;
        Ogre::SubMesh* second;

        ScriptUtils::_debugStack(vm);
        SCRIPT_ASSERT_RESULT(readSubMeshFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(readSubMeshFromUserData(vm, -1, &second));

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    void SubMeshUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getMaterialName, "getMaterialName");
        ScriptUtils::addFunction(vm, SubMeshToString, "_tostring");
        ScriptUtils::addFunction(vm, SubMeshCompare, "_cmp");

        sq_resetobject(&SubMeshDelegateTableObject);
        sq_getstackobj(vm, -1, &SubMeshDelegateTableObject);
        sq_addref(vm, &SubMeshDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger SubMeshUserData::SubMeshToString(HSQUIRRELVM vm){
        //Ogre::SubMesh* mesh;
        //SCRIPT_ASSERT_RESULT(readSubMeshFromUserData(vm, 1, &mesh));

        //TODO try and de-reference the pointer here to add more description.
        std::ostringstream stream;
        stream << "SubMesh";
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void SubMeshUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_PROG_TYPE_VERTEX
        @desc Vertex program.
        */
        //ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_VERTEX", (SQInteger)Ogre::GPT_VERTEX_PROGRAM);
    }
}
