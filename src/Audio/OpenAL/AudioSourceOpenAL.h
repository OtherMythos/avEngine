#pragma once

#include "Audio/AudioSource.h"

#include "AL/al.h"

namespace AV{
    class AudioSourceOpenAL : public AudioSource{
    public:
        AudioSourceOpenAL(AudioManager* manager);
        ~AudioSourceOpenAL();

        virtual void setAudioBuffer(AudioBufferPtr buffer) override;

        virtual void play() override;
        virtual void pause() override;
        virtual void stop() override;
        virtual void setPosition(const Ogre::Vector3& posVec) override;
        virtual void setLooping(bool looping) override;
        virtual void setVolume(float volume) override;
        virtual void setPitch(float pitch) override;
        virtual void seekSeconds(float seconds) override;
        virtual void setRolloff(float rolloff) override;
        virtual void setDirection(const Ogre::Vector3& dirVec) override;
        virtual void setVelocity(const Ogre::Vector3& vel) override;
        virtual void setAttenuationDistance(float ref, float max) override;

    private:
        ALuint mSource;
    };
}
