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

        *outProg = *((Ogre::VertexArrayObject**)pointer);

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

    SQInteger VertexArrayObjectUserData::setPrimitiveRange(HSQUIRRELVM vm){
        Ogre::VertexArrayObject* vao;
        SCRIPT_CHECK_RESULT(readVertexArrayObjectFromUserData(vm, 1, &vao));

        SQInteger primStart, primCount;
        sq_getinteger(vm, 2, &primStart);
        sq_getinteger(vm, 3, &primCount);

        //Ogre takes these as uint32 and range checks primStart + primCount
        //against the buffer. Negatives would wrap into that check as enormous
        //values whose sum can overflow back into a legal-looking range, so they
        //are rejected here rather than left to Ogre.
        if(primStart < 0) return sq_throwerror(vm, "Primitive start must be >= 0.");
        if(primCount < 0) return sq_throwerror(vm, "Primitive count must be >= 0.");

        WRAP_OGRE_ERROR(
            vao->setPrimitiveRange(static_cast<Ogre::uint32>(primStart), static_cast<Ogre::uint32>(primCount));
        )

        return 0;
    }

    SQInteger VertexArrayObjectUserData::getPrimitiveStart(HSQUIRRELVM vm){
        Ogre::VertexArrayObject* vao;
        SCRIPT_CHECK_RESULT(readVertexArrayObjectFromUserData(vm, 1, &vao));

        sq_pushinteger(vm, static_cast<SQInteger>(vao->getPrimitiveStart()));

        return 1;
    }

    SQInteger VertexArrayObjectUserData::getPrimitiveCount(HSQUIRRELVM vm){
        Ogre::VertexArrayObject* vao;
        SCRIPT_CHECK_RESULT(readVertexArrayObjectFromUserData(vm, 1, &vao));

        sq_pushinteger(vm, static_cast<SQInteger>(vao->getPrimitiveCount()));

        return 1;
    }

    void VertexArrayObjectUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, VertexArrayObjectToString, "_tostring");
        ScriptUtils::addFunction(vm, VertexArrayObjectCompare, "_cmp");
        /**SQFunction
        @name setPrimitiveRange
        @desc Limit how much of the buffer is drawn, without re-creating anything.
        For an indexed VAO this is the index start and count, otherwise the vertex
        start and count. Lets a pooled system (particles, dynamic geometry) draw
        only its live elements instead of padding the rest out of sight.
        @param1:Integer: First element to draw.
        @param2:Integer: How many elements to draw.
        */
        ScriptUtils::addFunction(vm, setPrimitiveRange, "setPrimitiveRange", 3, ".ii");
        /**SQFunction
        @name getPrimitiveStart
        @desc First element currently being drawn.
        @returns:Integer
        */
        ScriptUtils::addFunction(vm, getPrimitiveStart, "getPrimitiveStart");
        /**SQFunction
        @name getPrimitiveCount
        @desc How many elements are currently being drawn.
        @returns:Integer
        */
        ScriptUtils::addFunction(vm, getPrimitiveCount, "getPrimitiveCount");

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
