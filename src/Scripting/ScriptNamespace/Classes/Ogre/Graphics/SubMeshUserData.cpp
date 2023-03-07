#include "SubMeshUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/VertexArrayObjectUserData.h"

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

        *outProg = *((Ogre::SubMesh**)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger SubMeshUserData::clearMeshVAO(HSQUIRRELVM vm){
        Ogre::SubMesh* mesh;
        SCRIPT_ASSERT_RESULT(readSubMeshFromUserData(vm, 1, &mesh));

        if(sq_gettop(vm) >= 2){
            SQInteger vertexPass;
            sq_getinteger(vm, 3, &vertexPass);
            if(vertexPass < 0 || vertexPass >= Ogre::NumVertexPass){
                return sq_throwerror(vm, "Invalid vertex pass index.");
            }

            Ogre::VertexPass pass = static_cast<Ogre::VertexPass>(vertexPass);
            assert(pass == Ogre::VpNormal || pass == Ogre::VpShadow);
            mesh->mVao[pass].clear();
            return 0;
        }

        mesh->mVao[Ogre::VpNormal].clear();
        mesh->mVao[Ogre::VpShadow].clear();
    }

    SQInteger SubMeshUserData::pushMeshVAO(HSQUIRRELVM vm){
        Ogre::SubMesh* mesh;
        SCRIPT_ASSERT_RESULT(readSubMeshFromUserData(vm, 1, &mesh));

        Ogre::VertexArrayObject* obj;
        SCRIPT_CHECK_RESULT(VertexArrayObjectUserData::readVertexArrayObjectFromUserData(vm, 2, &obj));

        if(sq_gettop(vm) >= 3){
            SQInteger vertexPass;
            sq_getinteger(vm, 3, &vertexPass);
            if(vertexPass < 0 || vertexPass >= Ogre::NumVertexPass){
                return sq_throwerror(vm, "Invalid vertex pass index.");
            }

            Ogre::VertexPass pass = static_cast<Ogre::VertexPass>(vertexPass);
            assert(pass == Ogre::VpNormal || pass == Ogre::VpShadow);
            mesh->mVao[pass].push_back(obj);
            return 0;
        }

        mesh->mVao[Ogre::VpNormal].push_back(obj);
        mesh->mVao[Ogre::VpShadow].push_back(obj);

        return 0;
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
        ScriptUtils::addFunction(vm, pushMeshVAO, "pushMeshVAO", -2, ".ui");
        ScriptUtils::addFunction(vm, clearMeshVAO, "clearMeshVAO", -1, ".i");
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
        @name _VP_NORMAL
        @desc Vertex pass normal.
        */
        ScriptUtils::declareConstant(vm, "_VP_NORMAL", (SQInteger)Ogre::VpNormal);
        /**SQConstant
        @name _VP_SHADOW
        @desc Vertex pass shadow.
        */
        ScriptUtils::declareConstant(vm, "_VP_SHADOW", (SQInteger)Ogre::VpShadow);
    }
}
