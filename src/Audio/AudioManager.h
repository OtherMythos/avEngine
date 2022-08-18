#pragma once

#include "AudioTypes.h"

namespace AV{
    /**
    Manage the operation and execution of audio.

    This base class contains a null implementation,
    i.e the functions can be called but they produce no sound.
    */
    class AudioManager{
    public:
        AudioManager();
        ~AudioManager();

        virtual void play();
        virtual void pause();

        virtual AudioBufferPtr createAudioBuffer();
        virtual AudioSourcePtr createAudioSource(const std::string& audioPath, AudioSourceType type = AudioSourceType::Buffer);
    };
}
