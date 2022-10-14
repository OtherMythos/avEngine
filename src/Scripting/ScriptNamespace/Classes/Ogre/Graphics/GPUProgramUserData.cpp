#include "GPUProgramUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject GPUProgramUserData::GPUProgramDelegateTableObject;

    void GPUProgramUserData::GPUProgramToUserData(HSQUIRRELVM vm, Ogre::HighLevelGpuProgramPtr program){
        Ogre::HighLevelGpuProgramPtr* pointer = (Ogre::HighLevelGpuProgramPtr*)sq_newuserdata(vm, sizeof(Ogre::HighLevelGpuProgramPtr));
        memset(pointer, 0, sizeof(Ogre::HighLevelGpuProgramPtr));
        *pointer = program;

        sq_pushobject(vm, GPUProgramDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, GPUProgramTypeTag);
    }

    UserDataGetResult GPUProgramUserData::readGPUProgramFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::HighLevelGpuProgramPtr* outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != GPUProgramTypeTag){
            outProg->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((Ogre::HighLevelGpuProgramPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger GPUProgramUserData::getType(HSQUIRRELVM vm){
        Ogre::HighLevelGpuProgramPtr prog;
        SCRIPT_ASSERT_RESULT(readGPUProgramFromUserData(vm, 1, &prog));

        Ogre::GpuProgramType type = prog->getType();

        sq_pushinteger(vm, (SQInteger)type);

        return 1;
    }

    void GPUProgramUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getType, "getType");

        sq_resetobject(&GPUProgramDelegateTableObject);
        sq_getstackobj(vm, -1, &GPUProgramDelegateTableObject);
        sq_addref(vm, &GPUProgramDelegateTableObject);
        sq_pop(vm, 1);
    }

    void GPUProgramUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_PROG_TYPE_VERTEX
        @desc Vertex program.
        */
        ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_VERTEX", (SQInteger)Ogre::GPT_VERTEX_PROGRAM);
        /**SQConstant
        @name _GPU_PROG_TYPE_FRAGMENT
        @desc Fragment program.
        */
        ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_FRAGMENT", (SQInteger)Ogre::GPT_FRAGMENT_PROGRAM);
        /**SQConstant
        @name _GPU_PROG_TYPE_GEOMETRY
        @desc Geometry program.
        */
        ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_GEOMETRY", (SQInteger)Ogre::GPT_GEOMETRY_PROGRAM);
        /**SQConstant
        @name _GPU_PROG_TYPE_HULL
        @desc Hull program.
        */
        ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_HULL", (SQInteger)Ogre::GPT_HULL_PROGRAM);
        /**SQConstant
        @name _GPU_PROG_TYPE_DOMAIN
        @desc Domain program.
        */
        ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_DOMAIN", (SQInteger)Ogre::GPT_DOMAIN_PROGRAM);
        /**SQConstant
        @name _GPU_PROG_TYPE_COMPUTE
        @desc Compute program.
        */
        ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_COMPUTE", (SQInteger)Ogre::GPT_COMPUTE_PROGRAM);
    }
}
