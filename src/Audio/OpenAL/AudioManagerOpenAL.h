#pragma once

#include "Audio/AudioManager.h"
#include <string>

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
        void setListenerVelocity(Ogre::Vector3 velocity) override;
        void setListenerOrientation(float vec[6]) override;

        void setVolume(float volume) override;
#ifdef DEBUG_SERVER
        AudioManagerSnapshot debugSnapshot() const override;
#endif

    private:
        bool mSetupSuccesful;

        ALCdevice *mDevice;
        ALCcontext *mCtx;
        std::string mDeviceName;
#ifdef DEBUG_SERVER
        std::string mSetupError;
        void debugOperation(const char* operation);
#endif
    };
}
