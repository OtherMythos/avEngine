#pragma once

#include "Audio/AudioTypes.h"
#include "OgreVector3.h"
#include <string>
#ifdef DEBUG_SERVER
#include "AudioDebug.h"
#endif

namespace AV{
    class AudioManager;

    class AudioSource{
    public:
        AudioSource(AudioManager* manager);
        virtual ~AudioSource();

        virtual std::string getPlaybackState() const { return "unavailable"; }
        virtual double getDurationSeconds() const;
        virtual double getPositionSeconds() const { return 0; }
        virtual std::string getError() const;
        virtual std::string getAudioPath() const;
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
        std::string mSourcePath;

        AudioManager* mManager;

        Ogre::Vector3 mPos;

    public:
        AudioBufferPtr getAudioBuffer() const { return mAudioBuffer; }

        const Ogre::Vector3& getPosition() const { return mPos; }
#ifdef DEBUG_SERVER
        uint64 getDebugId() const { return mDebugId; }
        virtual AudioSourceSnapshot debugSnapshot() const { return AudioSourceSnapshot(); }
        virtual std::string debugPlaybackState() const { return debugSnapshot().state; }
    protected:
        void debugCommand(const char* command, const std::string& detail = "");
        uint64 mDebugId = 0;
#endif
    };
}
