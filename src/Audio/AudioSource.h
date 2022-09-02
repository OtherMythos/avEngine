#pragma once

#include "Audio/AudioTypes.h"
#include "OgreVector3.h"

namespace AV{
    class AudioManager;

    class AudioSource{
    public:
        AudioSource(AudioManager* manager);
        virtual ~AudioSource();

        virtual void play();
        virtual void pause();
        virtual void stop();
        virtual void setPosition(const Ogre::Vector3& posVec);
        virtual void setLooping(bool looping);
        virtual void setVolume(float volume);
        virtual void setPitch(float pitch);
        virtual void seekSeconds(float seconds);
        virtual void setRolloff(float rolloff);
        virtual void setDirection(const Ogre::Vector3& dirVec);
        virtual void setVelocity(const Ogre::Vector3& vel);
        virtual void setAttenuationDistance(float ref, float max);

        virtual void setAudioBuffer(AudioBufferPtr buffer);

    protected:
        AudioBufferPtr mAudioBuffer;

        AudioManager* mManager;

    public:
        AudioBufferPtr getAudioBuffer() const { return mAudioBuffer; }
    };
}
