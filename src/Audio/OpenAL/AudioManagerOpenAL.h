#pragma once

#include "Audio/AudioManager.h"

struct ALCdevice;
struct ALCcontext;

namespace AV{
    class AudioManagerOpenAL : public AudioManager{
    public:
        AudioManagerOpenAL();
        ~AudioManagerOpenAL();

        AudioSourcePtr createAudioSource(const std::string& audioPath, AudioSourceType type = AudioSourceType::Buffer);

        virtual void setup();
        virtual void shutdown();

    private:
        bool mSetupSuccesful;

        ALCdevice *mDevice;
        ALCcontext *mCtx;
    };
}
