#pragma once

#include <string>
#ifdef DEBUG_SERVER
#include "AudioDebug.h"
#endif

namespace AV{
    class AudioManager;

    class AudioBuffer{
    public:
        AudioBuffer(AudioManager* manager);
        virtual ~AudioBuffer();

        double getDurationSeconds() const { return mDurationSeconds; }
        const std::string& getError() const { return mError; }
        const std::string& getPath() const { return mPath; }
        virtual void play();
        virtual void pause();

        virtual void load(const std::string& path);

    protected:
        bool mBufferReady;
        double mDurationSeconds = -1;
        std::string mError, mPath;
        AudioManager* mManager;

    public:
        virtual unsigned int getBuffer() const { return 0; }
        bool isReady() const { return mBufferReady; }
#ifdef DEBUG_SERVER
        uint64 getDebugId() const { return mDebugId; }
        const AudioBufferDebugInfo& getDebugInfo() const { return mDebugInfo; }
    protected:
        void debugLoadFailure(const std::string& error);
        AudioBufferDebugInfo mDebugInfo;
        uint64 mDebugId = 0;
#endif
    };
}
