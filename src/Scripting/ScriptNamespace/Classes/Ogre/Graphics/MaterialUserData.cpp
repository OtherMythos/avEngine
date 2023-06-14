#include "MaterialUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramParametersUserData.h"

namespace AV{

    SQObject MaterialUserData::MaterialDelegateTableObject;

    void MaterialUserData::MaterialToUserData(HSQUIRRELVM vm, Ogre::MaterialPtr program){
        Ogre::MaterialPtr* pointer = (Ogre::MaterialPtr*)sq_newuserdata(vm, sizeof(Ogre::MaterialPtr));
        memset(pointer, 0, sizeof(Ogre::MaterialPtr));
        *pointer = program;

        sq_pushobject(vm, MaterialDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, MaterialTypeTag);
        sq_setreleasehook(vm, -1, MaterialObjectReleaseHook);
    }

    UserDataGetResult MaterialUserData::readMaterialFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MaterialPtr* outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != MaterialTypeTag){
            outProg->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((Ogre::MaterialPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger MaterialUserData::MaterialObjectReleaseHook(SQUserPointer p, SQInteger size){
        Ogre::MaterialPtr* ptr = static_cast<Ogre::MaterialPtr*>(p);
        ptr->reset();

        return 0;
    }

    SQInteger MaterialUserData::getName(HSQUIRRELVM vm){
        Ogre::MaterialPtr mat;
        SCRIPT_ASSERT_RESULT(readMaterialFromUserData(vm, 1, &mat));

        const Ogre::String& name = mat->getName();

        sq_pushstring(vm, name.c_str(), name.length());

        return 1;
    }

    SQInteger _getPassForMaterial(HSQUIRRELVM vm, Ogre::Pass** outPass){
        Ogre::MaterialPtr mat;
        SCRIPT_ASSERT_RESULT(MaterialUserData::readMaterialFromUserData(vm, 1, &mat));

        SQInteger passIdx;
        SQInteger techniqueIdx;

        sq_getinteger(vm, 2, &techniqueIdx);
        sq_getinteger(vm, 3, &passIdx);

        if(techniqueIdx >= mat->getNumTechniques() || techniqueIdx < 0){
            return sq_throwerror(vm, "Invalid technique id.");
        }
        Ogre::Technique* technique = mat->getTechnique(techniqueIdx);
        if(passIdx >= technique->getNumPasses() || passIdx < 0){
            return sq_throwerror(vm, "Invalid pass id.");
        }
        *outPass = technique->getPass(passIdx);

        return 0;
    }
    SQInteger MaterialUserData::getVertexProgram(HSQUIRRELVM vm){
        Ogre::Pass* outPass;
        SQInteger success = _getPassForMaterial(vm, &outPass);
        if(SQ_FAILED(success)) return success;

        Ogre::GpuProgramPtr gpuProgram = outPass->getVertexProgram();
        GPUProgramUserData::GPUProgramToUserData(vm, gpuProgram);

        return 1;
    }
    SQInteger MaterialUserData::getFragmentProgram(HSQUIRRELVM vm){
        Ogre::Pass* outPass;
        SQInteger success = _getPassForMaterial(vm, &outPass);
        if(SQ_FAILED(success)) return success;

        Ogre::GpuProgramPtr gpuProgram = outPass->getFragmentProgram();
        GPUProgramUserData::GPUProgramToUserData(vm, gpuProgram);

        return 1;
    }

    SQInteger MaterialUserData::getVertexProgramParameters(HSQUIRRELVM vm){
        Ogre::Pass* outPass;
        SQInteger success = _getPassForMaterial(vm, &outPass);
        if(SQ_FAILED(success)) return success;

        Ogre::GpuProgramParametersSharedPtr gpuProgram = outPass->getVertexProgramParameters();
        GPUProgramParametersUserData::GPUProgramParametersToUserData(vm, gpuProgram);

        return 1;
    }
    SQInteger MaterialUserData::getFragmentProgramParameters(HSQUIRRELVM vm){
        Ogre::Pass* outPass;
        SQInteger success = _getPassForMaterial(vm, &outPass);
        if(SQ_FAILED(success)) return success;

        Ogre::GpuProgramParametersSharedPtr gpuProgram = outPass->getFragmentProgramParameters();
        GPUProgramParametersUserData::GPUProgramParametersToUserData(vm, gpuProgram);

        return 1;
    }

    void MaterialUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getName, "getName");
        //TODO might need to get rid of this as I can't guarantee I'll need it if I had proper techniques and passes.
        ScriptUtils::addFunction(vm, getVertexProgram, "getVertexProgram", 3, ".ii");
        ScriptUtils::addFunction(vm, getFragmentProgram, "getFragmentProgram", 3, ".ii");
        ScriptUtils::addFunction(vm, getVertexProgramParameters, "getVertexProgramParameters", 3, ".ii");
        ScriptUtils::addFunction(vm, getFragmentProgramParameters, "getFragmentProgramParameters", 3, ".ii");
        ScriptUtils::addFunction(vm, getFragmentProgram, "setFragmentProgramConstant", 3, ".ii");

        sq_resetobject(&MaterialDelegateTableObject);
        sq_getstackobj(vm, -1, &MaterialDelegateTableObject);
        sq_addref(vm, &MaterialDelegateTableObject);
        sq_pop(vm, 1);
    }

    void MaterialUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_PROG_TYPE_VERTEX
        @desc Vertex program.
        */
        ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_VERTEX", (SQInteger)Ogre::GPT_VERTEX_PROGRAM);
    }
}
