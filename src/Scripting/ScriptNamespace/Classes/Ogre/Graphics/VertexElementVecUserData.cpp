#include "VertexElementVecUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Ogre.h"

#include <sstream>

namespace AV{

    SQObject VertexElementVecUserData::VertexElementVecDelegateTableObject;

    void VertexElementVecUserData::VertexElementVecObjectToUserData(HSQUIRRELVM vm, Ogre::VertexElement2Vec* vec){
        Ogre::VertexElement2Vec** pointer = (Ogre::VertexElement2Vec**)sq_newuserdata(vm, sizeof(Ogre::VertexElement2Vec*));
        *pointer = vec;

        sq_pushobject(vm, VertexElementVecDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, VertexElementVecTypeTag);
        sq_setreleasehook(vm, -1, VertexElementVecObjectReleaseHook);
    }


    UserDataGetResult VertexElementVecUserData::readVertexElementVecFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::VertexElement2Vec** outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != VertexElementVecTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((Ogre::VertexElement2Vec**)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger VertexElementVecUserData::VertexElementVecCompare(HSQUIRRELVM vm){
        Ogre::VertexElement2Vec* first;
        Ogre::VertexElement2Vec* second;

        SCRIPT_ASSERT_RESULT(readVertexElementVecFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(readVertexElementVecFromUserData(vm, -1, &second));

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    SQInteger VertexElementVecUserData::pushVertexElement(HSQUIRRELVM vm){
        Ogre::VertexElement2Vec* vec;
        SCRIPT_ASSERT_RESULT(readVertexElementVecFromUserData(vm, 1, &vec));

        SQInteger elementType;
        SQInteger elementSemantic;

        sq_getinteger(vm, 2, &elementType);
        sq_getinteger(vm, 3, &elementSemantic);

        vec->push_back(Ogre::VertexElement2(static_cast<Ogre::VertexElementType>(elementType), static_cast<Ogre::VertexElementSemantic>(elementSemantic)));

        return 0;
    }

    SQInteger VertexElementVecUserData::clear(HSQUIRRELVM vm){
        Ogre::VertexElement2Vec* vec;
        SCRIPT_ASSERT_RESULT(readVertexElementVecFromUserData(vm, 1, &vec));

        vec->clear();

        return 0;
    }

    void VertexElementVecUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, VertexElementVecToString, "_tostring");
        ScriptUtils::addFunction(vm, VertexElementVecCompare, "_cmp");
        ScriptUtils::addFunction(vm, pushVertexElement, "pushVertexElement", 3, ".ii");
        ScriptUtils::addFunction(vm, clear, "clear");



        sq_resetobject(&VertexElementVecDelegateTableObject);
        sq_getstackobj(vm, -1, &VertexElementVecDelegateTableObject);
        sq_addref(vm, &VertexElementVecDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger VertexElementVecUserData::VertexElementVecToString(HSQUIRRELVM vm){
        //Ogre::VertexElementVec* mesh;
        //SCRIPT_ASSERT_RESULT(readVertexElementVecFromUserData(vm, 1, &mesh));

        //TODO try and de-reference the pointer here to add more description.
        std::ostringstream stream;
        stream << "VertexElementVec";
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void VertexElementVecUserData::setupConstants(HSQUIRRELVM vm){
        //TODO add the constant definitions.
        ScriptUtils::declareConstant(vm, "_VET_FLOAT1", (SQInteger)Ogre::VET_FLOAT1);
        ScriptUtils::declareConstant(vm, "_VET_FLOAT2", (SQInteger)Ogre::VET_FLOAT2);
        ScriptUtils::declareConstant(vm, "_VET_FLOAT3", (SQInteger)Ogre::VET_FLOAT3);
        ScriptUtils::declareConstant(vm, "_VET_FLOAT4", (SQInteger)Ogre::VET_FLOAT4);
        ScriptUtils::declareConstant(vm, "_VET_COLOUR", (SQInteger)Ogre::VET_COLOUR);
        ScriptUtils::declareConstant(vm, "_VET_SHORT2", (SQInteger)Ogre::VET_SHORT2);
        ScriptUtils::declareConstant(vm, "_VET_SHORT4", (SQInteger)Ogre::VET_SHORT4);
        ScriptUtils::declareConstant(vm, "_VET_UBYTE4", (SQInteger)Ogre::VET_UBYTE4);
        ScriptUtils::declareConstant(vm, "_VET_COLOUR_ARGB", (SQInteger)Ogre::VET_COLOUR_ARGB);
        ScriptUtils::declareConstant(vm, "_VET_COLOUR_ABGR", (SQInteger)Ogre::VET_COLOUR_ABGR);
        ScriptUtils::declareConstant(vm, "_VET_DOUBLE1", (SQInteger)Ogre::VET_DOUBLE1);
        ScriptUtils::declareConstant(vm, "_VET_DOUBLE2", (SQInteger)Ogre::VET_DOUBLE2);
        ScriptUtils::declareConstant(vm, "_VET_DOUBLE3", (SQInteger)Ogre::VET_DOUBLE3);
        ScriptUtils::declareConstant(vm, "_VET_DOUBLE4", (SQInteger)Ogre::VET_DOUBLE4);
        ScriptUtils::declareConstant(vm, "_VET_USHORT1_DEPRECATED", (SQInteger)Ogre::VET_USHORT1_DEPRECATED);
        ScriptUtils::declareConstant(vm, "_VET_USHORT2", (SQInteger)Ogre::VET_USHORT2);
        ScriptUtils::declareConstant(vm, "_VET_USHORT3_DEPRECATED", (SQInteger)Ogre::VET_USHORT3_DEPRECATED);
        ScriptUtils::declareConstant(vm, "_VET_USHORT4", (SQInteger)Ogre::VET_USHORT4);
        ScriptUtils::declareConstant(vm, "_VET_INT1", (SQInteger)Ogre::VET_INT1);
        ScriptUtils::declareConstant(vm, "_VET_INT2", (SQInteger)Ogre::VET_INT2);
        ScriptUtils::declareConstant(vm, "_VET_INT3", (SQInteger)Ogre::VET_INT3);
        ScriptUtils::declareConstant(vm, "_VET_INT4", (SQInteger)Ogre::VET_INT4);
        ScriptUtils::declareConstant(vm, "_VET_UINT1", (SQInteger)Ogre::VET_UINT1);
        ScriptUtils::declareConstant(vm, "_VET_UINT2", (SQInteger)Ogre::VET_UINT2);
        ScriptUtils::declareConstant(vm, "_VET_UINT3", (SQInteger)Ogre::VET_UINT3);
        ScriptUtils::declareConstant(vm, "_VET_UINT4", (SQInteger)Ogre::VET_UINT4);
        ScriptUtils::declareConstant(vm, "_VET_BYTE4", (SQInteger)Ogre::VET_BYTE4);
        ScriptUtils::declareConstant(vm, "_VET_BYTE4_SNORM", (SQInteger)Ogre::VET_BYTE4_SNORM);
        ScriptUtils::declareConstant(vm, "_VET_UBYTE4_NORM", (SQInteger)Ogre::VET_UBYTE4_NORM);
        ScriptUtils::declareConstant(vm, "_VET_SHORT2_SNORM", (SQInteger)Ogre::VET_SHORT2_SNORM);
        ScriptUtils::declareConstant(vm, "_VET_SHORT4_SNORM", (SQInteger)Ogre::VET_SHORT4_SNORM);
        ScriptUtils::declareConstant(vm, "_VET_USHORT2_NORM", (SQInteger)Ogre::VET_USHORT2_NORM);
        ScriptUtils::declareConstant(vm, "_VET_USHORT4_NORM", (SQInteger)Ogre::VET_USHORT4_NORM);
        ScriptUtils::declareConstant(vm, "_VET_HALF2", (SQInteger)Ogre::VET_HALF2);
        ScriptUtils::declareConstant(vm, "_VET_HALF4", (SQInteger)Ogre::VET_HALF4);

        ScriptUtils::declareConstant(vm, "_VES_POSITION", (SQInteger)Ogre::VES_POSITION);
        ScriptUtils::declareConstant(vm, "_VES_BLEND_WEIGHTS", (SQInteger)Ogre::VES_BLEND_WEIGHTS);
        ScriptUtils::declareConstant(vm, "_VES_BLEND_INDICES", (SQInteger)Ogre::VES_BLEND_INDICES);
        ScriptUtils::declareConstant(vm, "_VES_NORMAL", (SQInteger)Ogre::VES_NORMAL);
        ScriptUtils::declareConstant(vm, "_VES_DIFFUSE", (SQInteger)Ogre::VES_DIFFUSE);
        ScriptUtils::declareConstant(vm, "_VES_SPECULAR", (SQInteger)Ogre::VES_SPECULAR);
        ScriptUtils::declareConstant(vm, "_VES_TEXTURE_COORDINATES", (SQInteger)Ogre::VES_TEXTURE_COORDINATES);
        ScriptUtils::declareConstant(vm, "_VES_BINORMAL", (SQInteger)Ogre::VES_BINORMAL);
        ScriptUtils::declareConstant(vm, "_VES_TANGENT", (SQInteger)Ogre::VES_TANGENT);
        ScriptUtils::declareConstant(vm, "_VES_BLEND_WEIGHTS2", (SQInteger)Ogre::VES_BLEND_WEIGHTS2);
        ScriptUtils::declareConstant(vm, "_VES_BLEND_INDICES", (SQInteger)Ogre::VES_BLEND_INDICES);
    }


    SQInteger VertexElementVecUserData::VertexElementVecObjectReleaseHook(SQUserPointer p, SQInteger size){

    }
}
