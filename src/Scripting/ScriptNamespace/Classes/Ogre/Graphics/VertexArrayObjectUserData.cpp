#include "VertexArrayObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Ogre.h"

#include <sstream>

namespace AV{

    SQObject VertexArrayObjectUserData::VertexArrayObjectDelegateTableObject;

    void VertexArrayObjectUserData::VertexArrayObjectToUserData(HSQUIRRELVM vm, Ogre::VertexArrayObject* VertexArrayObject){
        Ogre::VertexArrayObject** pointer = (Ogre::VertexArrayObject**)sq_newuserdata(vm, sizeof(Ogre::VertexArrayObject*));
        *pointer = VertexArrayObject;

        sq_pushobject(vm, VertexArrayObjectDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, VertexArrayObjectTypeTag);
    }

    UserDataGetResult VertexArrayObjectUserData::readVertexArrayObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::VertexArrayObject** outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != VertexArrayObjectTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = (Ogre::VertexArrayObject*)pointer;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger VertexArrayObjectUserData::VertexArrayObjectCompare(HSQUIRRELVM vm){
        Ogre::VertexArrayObject* first;
        Ogre::VertexArrayObject* second;

        ScriptUtils::_debugStack(vm);
        SCRIPT_ASSERT_RESULT(readVertexArrayObjectFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(readVertexArrayObjectFromUserData(vm, -1, &second));

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    void VertexArrayObjectUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, VertexArrayObjectToString, "_tostring");
        ScriptUtils::addFunction(vm, VertexArrayObjectCompare, "_cmp");

        sq_resetobject(&VertexArrayObjectDelegateTableObject);
        sq_getstackobj(vm, -1, &VertexArrayObjectDelegateTableObject);
        sq_addref(vm, &VertexArrayObjectDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger VertexArrayObjectUserData::VertexArrayObjectToString(HSQUIRRELVM vm){
        //Ogre::VertexArrayObject* mesh;
        //SCRIPT_ASSERT_RESULT(readVertexArrayObjectFromUserData(vm, 1, &mesh));

        //TODO try and de-reference the pointer here to add more description.
        std::ostringstream stream;
        stream << "VertexArrayObject";
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void VertexArrayObjectUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_PROG_TYPE_VERTEX
        @desc Vertex program.
        */
        //ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_VERTEX", (SQInteger)Ogre::GPT_VERTEX_PROGRAM);
    }
}
