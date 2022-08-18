#include "AudioNamespace.h"

#include "System/BaseSingleton.h"

#include "System/BaseSingleton.h"

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

    SQInteger AudioNamespace::newAudioSource(HSQUIRRELVM vm){
        const std::string path("path");
        AudioSourcePtr srcPtr = BaseSingleton::getAudioManager()->createAudioSource(path);

        AudioSourceUserData::audioSourceToUserData(vm, srcPtr);
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
    }
}
