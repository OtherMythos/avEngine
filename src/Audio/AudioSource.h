#pragma once

#include "Audio/AudioTypes.h"
#include "OgreVector3.h"
#ifdef DEBUG_SERVER
#include "AudioDebug.h"
#endif

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
        virtual void setRelative(bool relative);

        virtual void setAudioBuffer(AudioBufferPtr buffer);

    protected:
        AudioBufferPtr mAudioBuffer;

        AudioManager* mManager;

        Ogre::Vector3 mPos;

    public:
        AudioBufferPtr getAudioBuffer() const { return mAudioBuffer; }

        const Ogre::Vector3& getPosition() const { return mPos; }
#ifdef DEBUG_SERVER
        uint64_t getDebugId() const { return mDebugId; }
        virtual AudioSourceSnapshot debugSnapshot() const { return AudioSourceSnapshot(); }
        virtual std::string debugPlaybackState() const { return debugSnapshot().state; }
    protected:
        void debugCommand(const char* command, const std::string& detail = "");
        uint64_t mDebugId = 0;
#endif
    };
}
