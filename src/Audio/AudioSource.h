#pragma once

#include "Audio/AudioTypes.h"

namespace AV{
    class AudioSource{
    public:
        AudioSource();
        ~AudioSource();

        virtual void play();
        virtual void pause();

        virtual void setAudioBuffer(AudioBufferPtr buffer);

    private:
        AudioBufferPtr mAudioBuffer;

    public:
        AudioBufferPtr getAudioBuffer() const { return mAudioBuffer; }
    };
}
