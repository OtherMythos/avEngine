#include "GPUProgramParametersUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "GPUProgramHelper.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"

namespace AV{

    SQObject GPUProgramParametersUserData::GPUProgramParametersDelegateTableObject;

    void GPUProgramParametersUserData::GPUProgramParametersToUserData(HSQUIRRELVM vm, Ogre::GpuProgramParametersSharedPtr program){
        Ogre::GpuProgramParametersSharedPtr* pointer = (Ogre::GpuProgramParametersSharedPtr*)sq_newuserdata(vm, sizeof(Ogre::GpuProgramParametersSharedPtr));
        memset(pointer, 0, sizeof(Ogre::GpuProgramParametersSharedPtr));
        *pointer = program;

        sq_pushobject(vm, GPUProgramParametersDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, GPUProgramParametersTypeTag);
    }

    UserDataGetResult GPUProgramParametersUserData::readGPUProgramParametersFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::GpuProgramParametersSharedPtr* outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != GPUProgramParametersTypeTag){
            outProg->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((Ogre::GpuProgramParametersSharedPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    //TODO as this is a shared pointer it needs a release hook.
    SQInteger GPUProgramParametersUserData::setNamedConstant(HSQUIRRELVM vm){
        Ogre::GpuProgramParametersSharedPtr outPtr;
        SCRIPT_ASSERT_RESULT(readGPUProgramParametersFromUserData(vm, 1, &outPtr));

        SQInteger result = GPUProgramHelper::setNamedConstant(vm, outPtr);
        if(SQ_FAILED(result)) return result;

        return 1;
    }

    void GPUProgramParametersUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setNamedConstant, "setNamedConstant", 3, ".s i|f|u");

        sq_resetobject(&GPUProgramParametersDelegateTableObject);
        sq_getstackobj(vm, -1, &GPUProgramParametersDelegateTableObject);
        sq_addref(vm, &GPUProgramParametersDelegateTableObject);
        sq_pop(vm, 1);
    }
}
