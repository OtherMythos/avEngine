#pragma once

#include "Audio/AudioTypes.h"
#include "OgreVector3.h""

namespace AV{
    class AudioManager;

    class AudioSource{
    public:
        AudioSource(AudioManager* manager);
        virtual ~AudioSource();

        virtual void play();
        virtual void pause();
        virtual void setPosition(const Ogre::Vector3& posVec);

        virtual void setAudioBuffer(AudioBufferPtr buffer);

    protected:
        AudioBufferPtr mAudioBuffer;

        AudioManager* mManager;

    public:
        AudioBufferPtr getAudioBuffer() const { return mAudioBuffer; }
    };
}
