#include "GPUProgramUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

//TODO remove
#include "Scripting/ScriptNamespace/Classes/Ogre/Compositor/CompositorWorkspaceUserData.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuad.h"

//

#include "GPUProgramHelper.h"

#include "OgreGpuProgramParams.h"

namespace AV{

    SQObject GPUProgramUserData::GPUProgramDelegateTableObject;

    void GPUProgramUserData::GPUProgramToUserData(HSQUIRRELVM vm, Ogre::GpuProgramPtr program){
        Ogre::GpuProgramPtr* pointer = (Ogre::GpuProgramPtr*)sq_newuserdata(vm, sizeof(Ogre::GpuProgramPtr));
        memset(pointer, 0, sizeof(Ogre::GpuProgramPtr));
        *pointer = program;

        sq_pushobject(vm, GPUProgramDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, GPUProgramTypeTag);
    }

    UserDataGetResult GPUProgramUserData::readGPUProgramFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::GpuProgramPtr* outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != GPUProgramTypeTag){
            outProg->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((Ogre::GpuProgramPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger GPUProgramUserData::getType(HSQUIRRELVM vm){
        Ogre::GpuProgramPtr prog;
        SCRIPT_ASSERT_RESULT(readGPUProgramFromUserData(vm, 1, &prog));

        Ogre::GpuProgramType type = prog->getType();

        sq_pushinteger(vm, (SQInteger)type);

        return 1;
    }

    SQInteger GPUProgramUserData::getNumParameters(HSQUIRRELVM vm){
        Ogre::GpuProgramPtr prog;
        SCRIPT_ASSERT_RESULT(readGPUProgramFromUserData(vm, 1, &prog));

        size_t numParams = prog->getParameters().size();

        sq_pushinteger(vm, (SQInteger)numParams);

//        int constList = prog->getDefaultParameters()->getFloatConstantList().size();
//
//        for(float f : prog->getDefaultParameters()->getFloatConstantList()){
//            float ff = f;
//        }
//
//        Ogre::GpuConstantDefinition def = prog->getDefaultParameters()->getConstantDefinition("testVal");

        //prog->getDefaultParameters()->setConstant

        //TODO see if there's a way to query these values.
        //Add functions to be able to set values to these shared parameters.

        return 1;
    }

    SQInteger GPUProgramUserData::getParameterByIdx(HSQUIRRELVM vm){
        Ogre::GpuProgramPtr prog;
        SCRIPT_ASSERT_RESULT(readGPUProgramFromUserData(vm, 1, &prog));

        SQInteger targetIdx;
        sq_getinteger(vm, 2, &targetIdx);

        const Ogre::ParameterList& paramList = prog->getParameters();
        if(targetIdx >= paramList.size()){
            return sq_throwerror(vm, "Invalid idx for parameter.");
        }

        const Ogre::String& nameString = paramList[targetIdx].name;
        sq_pushstring(vm, nameString.c_str(), nameString.length());

        return 1;
    }

    SQInteger GPUProgramUserData::getNamedConstant(HSQUIRRELVM vm){

    }

    SQInteger GPUProgramUserData::setNamedConstant(HSQUIRRELVM vm){
        Ogre::GpuProgramPtr prog;
        SCRIPT_ASSERT_RESULT(readGPUProgramFromUserData(vm, 1, &prog));

        SQInteger result = GPUProgramHelper::setNamedConstant(vm, prog->getDefaultParameters());
        if(SQ_FAILED(result)) return result;

        return 0;
    }

    void GPUProgramUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getType, "getType");
        ScriptUtils::addFunction(vm, getNumParameters, "getNumParams");
        ScriptUtils::addFunction(vm, getParameterByIdx, "getParamByIdx", 2, ".i");
        ScriptUtils::addFunction(vm, setNamedConstant, "setNamedConstant", 3, ".s i|f|u");
        ScriptUtils::addFunction(vm, getNamedConstant, "getNamedConstant", 2, ".s");

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
