#include "DatablockUserData.h"

#include "OgreHlmsDatablock.h"
#include "OgreException.h"
#include "Scripting/ScriptNamespace/ScriptDefs.h"
#include "OgreResourceGroupManager.h"
#include "DatablockUnlitDelegate.h"
#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject DatablockUserData::pbsDelegateTableObject;
    SQObject DatablockUserData::unlitDelegateTableObject;

    SQInteger DatablockUserData::blockReleaseHook(SQUserPointer p, SQInteger size){

        return 0;
    }

    void DatablockUserData::DatablockPtrToUserData(HSQUIRRELVM vm, Ogre::HlmsDatablock* db){
        void* pointer = (void*)sq_newuserdata(vm, sizeof(Ogre::HlmsDatablock*));
        Ogre::HlmsDatablock** p = new (pointer) Ogre::HlmsDatablock*;
        *p = db;

        //sq_setreleasehook(vm, -1, blockReleaseHook);

        if(db->mType == Ogre::HLMS_PBS){
            sq_pushobject(vm, pbsDelegateTableObject);
        }else if(db->mType == Ogre::HLMS_UNLIT){
            sq_pushobject(vm, unlitDelegateTableObject);
        }else{
            assert(false); //Should not happen.
        }
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, datablockTypeTag);
    }

    SQInteger DatablockUserData::equalsDatablock(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db1 = 0;
        Ogre::HlmsDatablock* db2 = 0;

        SCRIPT_CHECK_RESULT(getPtrFromUserData(vm, -2, &db1));
        SCRIPT_CHECK_RESULT(getPtrFromUserData(vm, -1, &db2));

        sq_pushbool(vm, db1 == db2);

        return 1;
    }

    SQInteger DatablockUserData::cloneDatablock(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, -2, &db));

        const SQChar *name;
        sq_getstring(vm, -1, &name);

        Ogre::HlmsDatablock* newDb = db->clone(name);

        DatablockUserData::DatablockPtrToUserData(vm, newDb);

        return 1;
    }

    SQInteger DatablockUserData::getName(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, -1, &db));

        const Ogre::String* name = db->getNameStr();

        sq_pushstring(vm, name->c_str(), name->length());

        return 1;
    }

    static bool _readCustomPieceStage(HSQUIRRELVM vm, SQInteger idx, Ogre::CustomPieceStage::CustomPieceStage* outStage){
        SQInteger stage;
        sq_getinteger(vm, idx, &stage);
        if(stage < 0 || stage >= Ogre::CustomPieceStage::NumCustomPieceStages) return false;
        *outStage = static_cast<Ogre::CustomPieceStage::CustomPieceStage>(stage);
        return true;
    }

    SQInteger DatablockUserData::setCustomPieceFile(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, 1, &db));

        const SQChar *filename;
        sq_getstring(vm, 2, &filename);
        Ogre::CustomPieceStage::CustomPieceStage stage;
        if(!_readCustomPieceStage(vm, 3, &stage)) return sq_throwerror(vm, "Invalid custom piece stage.");
        const SQChar *group = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME.c_str();
        if(sq_gettop(vm) >= 4){
            sq_getstring(vm, 4, &group);
        }

        WRAP_OGRE_ERROR(db->setCustomPieceFile(filename, group, stage);)

        return 0;
    }

    SQInteger DatablockUserData::setCustomPieceCode(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, 1, &db));

        const SQChar *name;
        sq_getstring(vm, 2, &name);
        const SQChar *code;
        SQInteger codeLength;
        sq_getstringandsize(vm, 3, &code, &codeLength);
        Ogre::CustomPieceStage::CustomPieceStage stage;
        if(!_readCustomPieceStage(vm, 4, &stage)) return sq_throwerror(vm, "Invalid custom piece stage.");

        if(name[0] == '\0' && codeLength > 0){
            return sq_throwerror(vm, "Custom piece code requires a name.");
        }

        WRAP_OGRE_ERROR(db->setCustomPieceCodeFromMemory(name, Ogre::String(code, codeLength), stage);)

        return 0;
    }

    SQInteger DatablockUserData::getCustomPieceFile(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, 1, &db));

        Ogre::CustomPieceStage::CustomPieceStage stage;
        if(!_readCustomPieceStage(vm, 2, &stage)) return sq_throwerror(vm, "Invalid custom piece stage.");

        if(db->getCustomPieceFileIdHash(stage) == 0){
            sq_pushnull(vm);
            return 1;
        }
        const Ogre::String& filename = db->getCustomPieceFileStr(stage);
        sq_pushstring(vm, filename.c_str(), filename.length());

        return 1;
    }

    SQInteger DatablockUserData::setCustomProperties(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, 1, &db));

        Ogre::HlmsDatablock::CustomPropertyVec properties;
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, 2))){
            //-1 is the value and -2 is the key.
            if(sq_gettype(vm, -2) != OT_STRING){
                sq_pop(vm, 3);
                return sq_throwerror(vm, "Custom property names must be strings.");
            }
            const SQChar *key;
            sq_getstring(vm, -2, &key);

            SQObjectType t = sq_gettype(vm, -1);
            Ogre::int32 value = 0;
            if(t == OT_INTEGER){
                SQInteger val;
                sq_getinteger(vm, -1, &val);
                value = static_cast<Ogre::int32>(val);
            }else if(t == OT_BOOL){
                SQBool val;
                sq_getbool(vm, -1, &val);
                value = val ? 1 : 0;
            }else{
                sq_pop(vm, 3);
                return sq_throwerror(vm, "Custom property values must be integers or bools.");
            }
            properties.push_back(Ogre::HlmsDatablock::CustomProperty(key, value));

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);

        db->setCustomProperties(properties, true);

        return 0;
    }

    SQInteger DatablockUserData::getCustomProperties(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, 1, &db));

        const Ogre::HlmsDatablock::CustomPropertyVec& properties = db->getCustomProperties();
        sq_newtableex(vm, static_cast<SQInteger>(properties.size()));
        for(const Ogre::HlmsDatablock::CustomProperty& property : properties){
            sq_pushstring(vm, property.keyStr.c_str(), property.keyStr.length());
            sq_pushinteger(vm, property.value);
            sq_newslot(vm, -3, SQFalse);
        }

        return 1;
    }

    void DatablockUserData::setupSharedFunctions(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, setCustomPieceFile, "setCustomPieceFile", -3, ".sis");
        ScriptUtils::addFunction(vm, setCustomPieceCode, "setCustomPieceCode", 4, ".ssi");
        ScriptUtils::addFunction(vm, getCustomPieceFile, "getCustomPieceFile", 2, ".i");
        ScriptUtils::addFunction(vm, setCustomProperties, "setCustomProperties", 2, ".t");
        ScriptUtils::addFunction(vm, getCustomProperties, "getCustomProperties");
    }

    UserDataGetResult DatablockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::HlmsDatablock** outPtr){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != datablockTypeTag){
            *outPtr = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::HlmsDatablock** p = (Ogre::HlmsDatablock**)pointer;
        *outPtr = *p;

        return USER_DATA_GET_SUCCESS;
    }

    void DatablockUserData::setupDelegateTable(HSQUIRRELVM vm){
        ScriptUtils::setupDelegateTable(vm, &pbsDelegateTableObject, DatablockPbsDelegate::setupTable);
        ScriptUtils::setupDelegateTable(vm, &unlitDelegateTableObject, DatablockUnlitDelegate::setupTable);
    }
}
