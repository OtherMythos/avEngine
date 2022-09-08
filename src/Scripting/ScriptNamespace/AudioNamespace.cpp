#include "AudioNamespace.h"

#include "System/BaseSingleton.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "System/Util/PathUtils.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "Scripting/ScriptNamespace/Classes/Audio/AudioBufferUserData.h"

#include "Audio/AudioManager.h"
#include "Audio/AudioBuffer.h"
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
        SQInteger idx = ScriptGetterUtils::vector3Read(vm, &outVal);
        if(idx != 0) return idx;

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
        SQInteger idx = ScriptGetterUtils::vector3Read(vm, &outVal);
        if(idx != 0) return idx;

        BaseSingleton::getAudioManager()->setListenerVelocity(outVal);

        return 0;
    }

    SQInteger AudioNamespace::setListenerOrientation(HSQUIRRELVM vm){
        Ogre::Quaternion outQuat;
        SCRIPT_CHECK_RESULT(QuaternionUserData::readQuaternionFromUserData(vm, 2, &outQuat));

        Ogre::Vector3 forwardDir = outQuat * Ogre::Vector3::UNIT_X;
        Ogre::Vector3 upDir = outQuat * Ogre::Vector3::UNIT_Y;

        float vals[6] = {forwardDir.x, forwardDir.y, forwardDir.z, upDir.x, upDir.y, upDir.z};
        BaseSingleton::getAudioManager()->setListenerOrientation(vals);

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

    SQInteger AudioNamespace::newAudioSourceFromBuffer(HSQUIRRELVM vm){
        AudioBufferPtr bufPtr;
        SCRIPT_ASSERT_RESULT(AudioBufferUserData::readAudioBufferFromUserData(vm, 2, &bufPtr))

        AudioSourcePtr srcPtr = BaseSingleton::getAudioManager()->createAudioSourceFromBuffer(bufPtr);

        AudioSourceUserData::audioSourceToUserData(vm, srcPtr);
        return 1;
    }

    SQInteger AudioNamespace::newSoundBuffer(HSQUIRRELVM vm){
        AudioBufferPtr bufPtr = BaseSingleton::getAudioManager()->createAudioBuffer();

        if(sq_gettop(vm) == 2){
            const SQChar *filePath;
            sq_getstring(vm, 2, &filePath);
            std::string outString;
            formatResToPath(filePath, outString);

            bufPtr->load(outString);
        }

        AudioBufferUserData::audioBufferToUserData(vm, bufPtr);

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

    SQInteger AudioNamespace::getNumAudioSources(HSQUIRRELVM vm){
        int audioSources = BaseSingleton::getAudioManager()->getNumAudioSources();

        sq_pushinteger(vm, audioSources);

        return 1;
    }
    SQInteger AudioNamespace::getNumAudioBuffers(HSQUIRRELVM vm){
        int audioBuffers = BaseSingleton::getAudioManager()->getNumAudioBuffers();

        sq_pushinteger(vm, audioBuffers);

        return 1;
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
        @name newSourceFromBuffer
        @desc Create a new sound buffer, passing in a pre-existing buffer.
        */
        ScriptUtils::addFunction(vm, newAudioSourceFromBuffer, "newSourceFromBuffer");
        /**SQFunction
        @name newSoundBuffer
        @desc Create a new sound buffer. This buffer can be re-used to create many audio sources.
        */
        ScriptUtils::addFunction(vm, newSoundBuffer, "newSoundBuffer", -1, ".s");
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
        @name setListenerOrientation
        @desc Set the orientation of the listener.
        @param1:Quaternion:Orientation of the listener.
        */
        ScriptUtils::addFunction(vm, setListenerOrientation, "setListenerOrientation", 2, ".u");
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
        @name getNumAudioSources
        @desc Get the number of audio sources which currently exist..
        */
        ScriptUtils::addFunction(vm, getNumAudioSources, "getNumAudioSources");
        /**SQFunction
        @name getNumAudioBuffers
        @desc Get the number of audio buffers which currently exist..
        */
        ScriptUtils::addFunction(vm, getNumAudioBuffers, "getNumAudioBuffers");
    }
}
