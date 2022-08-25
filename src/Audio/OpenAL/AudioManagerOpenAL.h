#pragma once

#include "Audio/AudioManager.h"

struct ALCdevice;
struct ALCcontext;

namespace AV{
    class AudioManagerOpenAL : public AudioManager{
    public:
        AudioManagerOpenAL();
        ~AudioManagerOpenAL();

        AudioBufferPtr createAudioBuffer() override;
        AudioSourcePtr createAudioSource(const std::string& audioPath, AudioSourceType type = AudioSourceType::Buffer) override;
        AudioSourcePtr createAudioSourceFromBuffer(AudioBufferPtr bufPtr) override;

        void setup() override;
        void shutdown() override;

        void setListenerPosition(Ogre::Vector3 pos) override;
        Ogre::Vector3 getListenerPosition() const override;
        void setListenerVelocity(Ogre::Vector3 velocity) override;
        Ogre::Vector3 getListenerVelocity() const override;

        float getVolume() const override;
        void setVolume(float volume) override;

    private:
        bool mSetupSuccesful;

        ALCdevice *mDevice;
        ALCcontext *mCtx;
    };
}
