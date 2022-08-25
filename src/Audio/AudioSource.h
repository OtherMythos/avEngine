#pragma once

#include "Audio/AudioTypes.h"

namespace AV{
    class AudioManager;

    class AudioSource{
    public:
        AudioSource(AudioManager* manager);
        virtual ~AudioSource();

        virtual void play();
        virtual void pause();

        virtual void setAudioBuffer(AudioBufferPtr buffer);

    protected:
        AudioBufferPtr mAudioBuffer;

        AudioManager* mManager;

    public:
        AudioBufferPtr getAudioBuffer() const { return mAudioBuffer; }
    };
}
