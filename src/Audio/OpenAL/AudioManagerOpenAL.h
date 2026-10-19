#pragma once

#include "Audio/AudioManager.h"
#include <string>
#include <vector>
#include "AL/alc.h"
#include "AL/alext.h"

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
        void setSimulationPaused(bool paused) override;
        void simulationTick(int rate) override;
        const std::vector<float>* simulationSamples() const override;

        void setListenerPosition(Ogre::Vector3 pos) override;
        Ogre::Vector3 getListenerPosition() const override;
        void setListenerVelocity(Ogre::Vector3 velocity) override;
        Ogre::Vector3 getListenerVelocity() const override;
        void setListenerOrientation(float vec[6]) override;

        float getVolume() const override;
        void setVolume(float volume) override;
#ifdef DEBUG_SERVER
        AudioManagerSnapshot debugSnapshot() const override;
#endif

    private:
        bool mSetupSuccesful;
        bool mSimulationPaused = false;
        bool mLoopback = false;
        LPALCDEVICEPAUSESOFT mPauseDevice = nullptr;
        LPALCDEVICERESUMESOFT mResumeDevice = nullptr;
        LPALCRENDERSAMPLESSOFT mRenderSamples = nullptr;
        unsigned mSampleRemainder = 0;
        std::vector<float> mMixedSamples;


        ALCdevice *mDevice;
        ALCcontext *mCtx;
        std::string mDeviceName;
#ifdef DEBUG_SERVER
        std::string mSetupError;
        void debugOperation(const char* operation);
#endif
    };
}
