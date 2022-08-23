#pragma once

#include "Audio/AudioSource.h"

#include "AL/al.h"

namespace AV{
    class AudioSourceOpenAL : public AudioSource{
    public:
        AudioSourceOpenAL();
        ~AudioSourceOpenAL();

        virtual void setAudioBuffer(AudioBufferPtr buffer);

        virtual void play();
        virtual void pause();

    private:
        ALuint mSource;
    };
}
