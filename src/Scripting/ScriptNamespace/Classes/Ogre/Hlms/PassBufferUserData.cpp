#include "PassBufferUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "System/OgreSetup/CustomHLMS/AVHlmsListenerDispatch.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"

#include "OgreColourValue.h"
#include "OgreVector3.h"

#include <sqstdblob.h>

namespace AV{

    SQObject PassBufferUserData::PassBufferDelegateTableObject;

    void PassBufferUserData::passBufferToUserData(HSQUIRRELVM vm, Ogre::AVHlmsListenerDispatch* dispatch){
        Ogre::AVHlmsListenerDispatch** pointer = (Ogre::AVHlmsListenerDispatch**)sq_newuserdata(vm, sizeof(Ogre::AVHlmsListenerDispatch*));
        *pointer = dispatch;

        sq_pushobject(vm, PassBufferDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, PassBufferTypeTag);
    }

    UserDataGetResult PassBufferUserData::readPassBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::AVHlmsListenerDispatch** outDispatch){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != PassBufferTypeTag){
            *outDispatch = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outDispatch = *((Ogre::AVHlmsListenerDispatch**)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger PassBufferUserData::setFloat(HSQUIRRELVM vm){
        Ogre::AVHlmsListenerDispatch* dispatch;
        SCRIPT_CHECK_RESULT(readPassBufferFromUserData(vm, 1, &dispatch));

        SQInteger index;
        sq_getinteger(vm, 2, &index);
        SQFloat value;
        sq_getfloat(vm, 3, &value);

        if(index < 0) return sq_throwerror(vm, "Pass buffer index must be >= 0.");
        dispatch->setScriptFloat(static_cast<size_t>(index), static_cast<float>(value));

        return 0;
    }

    SQInteger PassBufferUserData::setData(HSQUIRRELVM vm){
        Ogre::AVHlmsListenerDispatch* dispatch;
        SCRIPT_CHECK_RESULT(readPassBufferFromUserData(vm, 1, &dispatch));

        //setData(blob) writes from offset 0; setData(offset, blob) from `offset`.
        SQInteger top = sq_gettop(vm);
        SQInteger offset = 0;
        SQInteger blobStackIdx = 2;
        if(top >= 3){
            sq_getinteger(vm, 2, &offset);
            blobStackIdx = 3;
        }
        if(offset < 0) return sq_throwerror(vm, "Pass buffer offset must be >= 0.");

        SQUserPointer blobData = 0;
        if(SQ_FAILED(sqstd_getblob(vm, blobStackIdx, &blobData))){
            return sq_throwerror(vm, "setData expects a blob of floats.");
        }
        SQInteger blobSize = sqstd_getblobsize(vm, blobStackIdx);
        dispatch->setScriptFloats(static_cast<size_t>(offset), static_cast<const float*>(blobData), static_cast<size_t>(blobSize) / sizeof(float));

        return 0;
    }

    SQInteger PassBufferUserData::setVec3(HSQUIRRELVM vm){
        Ogre::AVHlmsListenerDispatch* dispatch;
        SCRIPT_CHECK_RESULT(readPassBufferFromUserData(vm, 1, &dispatch));

        SQInteger index;
        sq_getinteger(vm, 2, &index);
        Ogre::Vector3 vec;
        SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &vec));

        if(index < 0) return sq_throwerror(vm, "Pass buffer index must be >= 0.");
        const float values[3] = { static_cast<float>(vec.x), static_cast<float>(vec.y), static_cast<float>(vec.z) };
        dispatch->setScriptFloats(static_cast<size_t>(index), values, 3);

        return 0;
    }

    SQInteger PassBufferUserData::setVec4(HSQUIRRELVM vm){
        Ogre::AVHlmsListenerDispatch* dispatch;
        SCRIPT_CHECK_RESULT(readPassBufferFromUserData(vm, 1, &dispatch));

        SQInteger index;
        sq_getinteger(vm, 2, &index);
        Ogre::ColourValue col;
        SCRIPT_CHECK_RESULT(ColourValueUserData::readColourValueFromUserData(vm, 3, &col));

        if(index < 0) return sq_throwerror(vm, "Pass buffer index must be >= 0.");
        const float values[4] = { col.r, col.g, col.b, col.a };
        dispatch->setScriptFloats(static_cast<size_t>(index), values, 4);

        return 0;
    }

    SQInteger PassBufferUserData::getSize(HSQUIRRELVM vm){
        Ogre::AVHlmsListenerDispatch* dispatch;
        SCRIPT_CHECK_RESULT(readPassBufferFromUserData(vm, 1, &dispatch));

        sq_pushinteger(vm, static_cast<SQInteger>(dispatch->getScriptBufferSize()));
        return 1;
    }

    void PassBufferUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setFloat, "setFloat", 3, ".in");
        ScriptUtils::addFunction(vm, setData, "setData", -2, ".i|x x|o");
        ScriptUtils::addFunction(vm, setVec3, "setVec3", 3, ".iu");
        ScriptUtils::addFunction(vm, setVec4, "setVec4", 3, ".iu");
        ScriptUtils::addFunction(vm, getSize, "getSize", 1, ".");

        sq_resetobject(&PassBufferDelegateTableObject);
        sq_getstackobj(vm, -1, &PassBufferDelegateTableObject);
        sq_addref(vm, &PassBufferDelegateTableObject);
        sq_pop(vm, 1);
    }
}
