#include "PassPropertiesUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "System/OgreSetup/CustomHLMS/AVHlmsListenerDispatch.h"

#include "OgreIdString.h"

namespace AV{

    SQObject PassPropertiesUserData::PassPropertiesDelegateTableObject;

    void PassPropertiesUserData::passPropertiesToUserData(HSQUIRRELVM vm, Ogre::AVHlmsListenerDispatch* dispatch, uint32_t passIdentifier){
        PassPropertiesData* pointer = (PassPropertiesData*)sq_newuserdata(vm, sizeof(PassPropertiesData));
        pointer->dispatch = dispatch;
        pointer->identifier = passIdentifier;

        sq_pushobject(vm, PassPropertiesDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, PassPropertiesTypeTag);
    }

    UserDataGetResult PassPropertiesUserData::_read(HSQUIRRELVM vm, SQInteger stackInx, PassPropertiesData* outData){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != PassPropertiesTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outData = *((PassPropertiesData*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger PassPropertiesUserData::setProperty(HSQUIRRELVM vm){
        PassPropertiesData data;
        SCRIPT_CHECK_RESULT(_read(vm, 1, &data));

        const SQChar* name;
        sq_getstring(vm, 2, &name);
        SQInteger value;
        sq_getinteger(vm, 3, &value);

        data.dispatch->setPassProperty(data.identifier, Ogre::IdString(name), static_cast<Ogre::int32>(value));

        return 0;
    }

    SQInteger PassPropertiesUserData::clearProperty(HSQUIRRELVM vm){
        PassPropertiesData data;
        SCRIPT_CHECK_RESULT(_read(vm, 1, &data));

        const SQChar* name;
        sq_getstring(vm, 2, &name);

        data.dispatch->clearPassProperty(data.identifier, Ogre::IdString(name));

        return 0;
    }

    void PassPropertiesUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setProperty, "setProperty", 3, ".si");
        ScriptUtils::addFunction(vm, clearProperty, "clearProperty", 2, ".s");

        sq_resetobject(&PassPropertiesDelegateTableObject);
        sq_getstackobj(vm, -1, &PassPropertiesDelegateTableObject);
        sq_addref(vm, &PassPropertiesDelegateTableObject);
        sq_pop(vm, 1);
    }
}
