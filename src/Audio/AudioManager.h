#pragma once

#include "AudioTypes.h"

#include "OgreVector3.h"
#include <vector>
#include <set>
#ifdef DEBUG_SERVER
#include "AudioDebug.h"
#endif

namespace AV{
    class AudioSource;
    class AudioBuffer;

    /**
    Manage the operation and execution of audio.

    This base class contains a null implementation,
    i.e the functions can be called but they produce no sound.
    */
    class AudioManager{
        friend AudioSource;
        friend AudioBuffer;
    public:
        AudioManager();
        virtual ~AudioManager();

        virtual void setup() { }
        virtual void shutdown();
        virtual void play() { }
        virtual void pause() { }
        virtual void setSimulationPaused(bool paused) { }
        virtual void simulationTick(int rate) { }
        virtual const std::vector<float>* simulationSamples() const { return nullptr; }

        //The listener state is kept here rather than read back from the backend, so values round trip
        //even when no audio device could be opened or audio is disabled. Implementations record the
        //value through these, then forward it on.
        virtual void setListenerPosition(Ogre::Vector3 pos) { mListenerPosition = pos; }
        Ogre::Vector3 getListenerPosition() const { return mListenerPosition; }
        virtual void setListenerVelocity(Ogre::Vector3 velocity) { mListenerVelocity = velocity; }
        Ogre::Vector3 getListenerVelocity() const { return mListenerVelocity; }

        virtual void setListenerOrientation(float vec[6]);

        float getVolume() const { return mVolume; }
        virtual void setVolume(float volume);

        bool isBufferAttached(const AudioBuffer* buffer) const;
        virtual AudioBufferPtr createAudioBuffer();
        virtual AudioSourcePtr createAudioSource(const std::string& audioPath, AudioSourceType type = AudioSourceType::Buffer);
        virtual AudioSourcePtr createAudioSourceFromBuffer(AudioBufferPtr bufPtr);

    private:
        int mNumAudioBuffers;
        int mNumAudioSources;
        bool mSetup;

    protected:
        std::set<AudioSource*> mSources;
        std::set<AudioBuffer*> mBuffers;

        Ogre::Vector3 mListenerPosition;
        Ogre::Vector3 mListenerVelocity;
        //Forward then up, as OpenAL expects it.
        float mListenerOrientation[6];
        float mVolume;

    private:

    public:
        int getNumAudioBuffers() const { return mNumAudioBuffers; }
        int getNumAudioSources() const { return mNumAudioSources; }
        bool isSetup() const { return mSetup; }
#ifdef DEBUG_SERVER
        AudioDebugState& debugState() { return mDebugState; }
        virtual AudioManagerSnapshot debugSnapshot() const { return AudioManagerSnapshot(); }
    private:
        AudioDebugState mDebugState;
#endif

    protected:
        void setSetup(bool setup) { mSetup = setup; }

    };
}
