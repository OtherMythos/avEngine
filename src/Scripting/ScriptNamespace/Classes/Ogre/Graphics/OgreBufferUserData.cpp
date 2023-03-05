#include "OgreBufferUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Ogre.h"

#include <sstream>

namespace AV{

    SQObject OgreBufferUserData::OgreBufferDelegateTableObject;

    void OgreBufferUserData::OgreBufferToUserData(HSQUIRRELVM vm, const OgreBufferData* ogreBuffer){
        OgreBufferData* pointer = (OgreBufferData*)sq_newuserdata(vm, sizeof(OgreBufferData));
        *pointer = *ogreBuffer;

        sq_pushobject(vm, OgreBufferDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, OgreBufferObjectTypeTag);
    }

    UserDataGetResult OgreBufferUserData::readOgreBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, OgreBufferData* outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != OgreBufferObjectTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((const OgreBufferData*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger OgreBufferUserData::OgreBufferCompare(HSQUIRRELVM vm){
        /*const OgreBufferData* first;
        const OgreBufferData* second;

        ScriptUtils::_debugStack(vm);
        SCRIPT_ASSERT_RESULT(readOgreBufferFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(readOgreBufferFromUserData(vm, -1, &second));

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }*/
        return 1;
    }

    void OgreBufferUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, OgreBufferToString, "_tostring");
        ScriptUtils::addFunction(vm, OgreBufferCompare, "_cmp");

        sq_resetobject(&OgreBufferDelegateTableObject);
        sq_getstackobj(vm, -1, &OgreBufferDelegateTableObject);
        sq_addref(vm, &OgreBufferDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger OgreBufferUserData::OgreBufferToString(HSQUIRRELVM vm){
        OgreBufferData outData;
        SCRIPT_ASSERT_RESULT(readOgreBufferFromUserData(vm, 1, &outData));

        //TODO try and de-reference the pointer here to add more description.
        std::ostringstream stream;
        stream << (outData.bufType == OgreBufferType::vertexBuffer ? "VertexBuffer" : "IndexBuffer");
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void OgreBufferUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_PROG_TYPE_VERTEX
        @desc Vertex program.
        */
        //ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_VERTEX", (SQInteger)Ogre::GPT_VERTEX_PROGRAM);
    }
}
