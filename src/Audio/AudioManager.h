#pragma once

#include "AudioTypes.h"

#include "OgreVector3.h"

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
        ~AudioManager();

        virtual void setup() { }
        virtual void shutdown() { }
        virtual void play() { }
        virtual void pause() { }

        virtual void setListenerPosition(Ogre::Vector3 pos) { }
        virtual Ogre::Vector3 getListenerPosition() const { return Ogre::Vector3::ZERO; }
        virtual void setListenerVelocity(Ogre::Vector3 velocity) { }
        virtual Ogre::Vector3 getListenerVelocity() const { return Ogre::Vector3::ZERO; }

        virtual void setListenerOrientation(float vec[6]) {  }

        virtual float getVolume() const { return 1.0f; }
        virtual void setVolume(float volume) { };

        virtual AudioBufferPtr createAudioBuffer();
        virtual AudioSourcePtr createAudioSource(const std::string& audioPath, AudioSourceType type = AudioSourceType::Buffer);
        virtual AudioSourcePtr createAudioSourceFromBuffer(AudioBufferPtr bufPtr);

    private:
        int mNumAudioBuffers;
        int mNumAudioSources;

    public:
        int getNumAudioBuffers() const { return mNumAudioBuffers; }
        int getNumAudioSources() const { return mNumAudioSources; }

    };
}
