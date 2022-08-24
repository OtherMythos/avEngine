#include "AudioNamespace.h"

#include "System/BaseSingleton.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "System/Util/PathUtils.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

#include "Audio/AudioManager.h"
#include "Scripting/ScriptNamespace/Classes/Audio/AudioSourceUserData.h"

namespace AV{

    SQInteger AudioNamespace::play(HSQUIRRELVM vm){
        BaseSingleton::getAudioManager()->play();

        return 0;
    }

    SQInteger AudioNamespace::pause(HSQUIRRELVM vm){
        BaseSingleton::getAudioManager()->pause();

        return 0;
    }

    SQInteger AudioNamespace::getListenerPosition(HSQUIRRELVM vm){
        const Ogre::Vector3 vec = BaseSingleton::getAudioManager()->getListenerPosition();

        Vector3UserData::vector3ToUserData(vm, vec);

        return 1;
    }

    SQInteger AudioNamespace::setListenerPosition(HSQUIRRELVM vm){
        Ogre::Vector3 outVal;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &outVal));

        BaseSingleton::getAudioManager()->setListenerPosition(outVal);

        return 0;
    }

    SQInteger AudioNamespace::getListenerVelocity(HSQUIRRELVM vm){
        const Ogre::Vector3 vec = BaseSingleton::getAudioManager()->getListenerVelocity();

        Vector3UserData::vector3ToUserData(vm, vec);

        return 1;
    }

    SQInteger AudioNamespace::setListenerVelocity(HSQUIRRELVM vm){
        Ogre::Vector3 outVal;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &outVal));

        BaseSingleton::getAudioManager()->setListenerVelocity(outVal);

        return 0;
    }

    SQInteger AudioNamespace::newAudioSource(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);

        std::string outString;
        formatResToPath(filePath, outString);

        AudioSourcePtr srcPtr = BaseSingleton::getAudioManager()->createAudioSource(outString);

        AudioSourceUserData::audioSourceToUserData(vm, srcPtr);
        return 1;
    }

    SQInteger AudioNamespace::getVolume(HSQUIRRELVM vm){
        float audio = BaseSingleton::getAudioManager()->getVolume();

        sq_pushfloat(vm, audio);
        return 1;
    }

    SQInteger AudioNamespace::setVolume(HSQUIRRELVM vm){
        SQFloat floatVal;
        sq_getfloat(vm, 2, &floatVal);
        if(floatVal < 0.0 || floatVal > 1.0) return sq_throwerror(vm, (std::string("Volume must be in range 0-1. Provided value ") + std::to_string(floatVal)).c_str());

        BaseSingleton::getAudioManager()->setVolume(floatVal);

        return 0;
    }

    /**SQNamespace
    @name _audio
    @desc A namespace to control audio.
    */
    void AudioNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name play
        @desc Play audio output
        */
        ScriptUtils::addFunction(vm, play, "play");
        /**SQFunction
        @name pause
        @desc Pause audio output
        */
        ScriptUtils::addFunction(vm, pause, "pause");
        /**SQFunction
        @name newSource
        @desc Create a new audio source.
        */
        ScriptUtils::addFunction(vm, newAudioSource, "newSource");
        /**SQFunction
        @name setListenerPosition
        @desc Set the position of the listener
        */
        ScriptUtils::addFunction(vm, setListenerPosition, "setListenerPosition", -2, ".n|unn");
        /**SQFunction
        @name getListenerPosition
        @desc Get the position of the listener
        */
        ScriptUtils::addFunction(vm, getListenerPosition, "getListenerPosition");
        /**SQFunction
        @name getListenerVelocity
        @desc Get the velocity of the listener
        @returns A Vec3 containing the velocity.
        */
        ScriptUtils::addFunction(vm, getListenerVelocity, "getListenerVelocity");
        /**SQFunction
        @name setListenerVelocity
        @desc Set velocity for the listener.
        @param1:Vec3:Vector3 containing the velocity.
        */
        ScriptUtils::addFunction(vm, setListenerVelocity, "setListenerVelocity", -2, ".n|unn");
        /**SQFunction
        @name setVolume
        @desc Set the master volume.
        @param1:float:Float decimal representing volume.
        */
        ScriptUtils::addFunction(vm, setVolume, "setVolume", 2, ".f");
        /**SQFunction
        @name getVolume
        @desc Get the master volume.
        @returns Volume as a float decimal.
        */
        ScriptUtils::addFunction(vm, getVolume, "getVolume");
        /**SQFunction
        @name setVolume
        @desc Set the master volume.
        @param1:float:Float decimal representing volume.
        */
        ScriptUtils::addFunction(vm, setVolume, "setVolume", 2, ".f");
    }
}
