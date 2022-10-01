#include "AudioSourceComponentNamespace.h"

#include "Logger/Log.h"
#include "World/Entity/Logic/AudioSourceComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/Entity/EntityUserData.h"
#include "Scripting/ScriptNamespace/Classes/Audio/AudioSourceUserData.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger AudioSourceComponentNamespace::add(HSQUIRRELVM vm){
        SQInteger size = sq_gettop(vm);

        eId id;
        AudioSourcePtr a = 0;
        AudioSourcePtr b = 0;

        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, 2, &id));
        //If null then continue as normal. Adding the component without an audio source can be useful.
        if(sq_gettype(vm, 3) != OT_NULL){
            SCRIPT_CHECK_RESULT(AudioSourceUserData::readAudioSourceFromUserData(vm, 3, &a));
        }
        if(size == 3) {}
        else if(size == 4){
            if(sq_gettype(vm, 4) != OT_NULL) SCRIPT_CHECK_RESULT(AudioSourceUserData::readAudioSourceFromUserData(vm, 4, &b));
        }else{
            return sq_throwerror(vm, "Incorrect number of arguments");
        }

        AudioSourceComponentLogic::add(id, a, b, true, size >= 4);

        return 0;
    }

    SQInteger AudioSourceComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        AudioSourceComponentLogic::remove(id);

        return 0;
    }

    SQInteger AudioSourceComponentNamespace::get(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, 2, &id));

        SQInteger target;
        sq_getinteger(vm, 3, &target);
        if(target < 0 || target > 1) return sq_throwerror(vm, "Target must be in range of 0 and 1");

        AudioSourcePtr ptr = AudioSourceComponentLogic::getAudioSource(id, static_cast<uint8>(target));
        AudioSourceUserData::audioSourceToUserData(vm, ptr);

        return 1;
    }

    SQInteger AudioSourceComponentNamespace::set(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, 2, &id));

        SQInteger target;
        sq_getinteger(vm, 3, &target);
        if(target < 0 || target > 1) return sq_throwerror(vm, "Target must be in range of 0 and 1");

        AudioSourcePtr a = 0;
        SCRIPT_CHECK_RESULT(AudioSourceUserData::readAudioSourceFromUserData(vm, 4, &a));

        bool result = AudioSourceComponentLogic::setAudioSource(id, static_cast<uint8>(target), a);
        if(!result) return sq_throwerror(vm, "Unable to set audio source. Are you sure this entity has an audio source component?");

        return 0;
    }

    void AudioSourceComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("audio"), -1);
        sq_newtableex(vm, 3);

        ScriptUtils::addFunction(vm, add, "add", -3, ".u u|o u|o");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".u");

        ScriptUtils::addFunction(vm, get, "get", 3, ".ui");
        ScriptUtils::addFunction(vm, set, "set", 4, ".uiu");

        sq_newslot(vm, -3, false);
    }
}
