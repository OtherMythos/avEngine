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

        virtual void play();
        virtual void pause();

        virtual void load(const std::string& path);

    protected:
        bool mBufferReady;
        AudioManager* mManager;

    public:
        virtual unsigned int getBuffer() const { return 0; }
        bool isReady() const { return mBufferReady; }
#ifdef DEBUG_SERVER
        uint64_t getDebugId() const { return mDebugId; }
        const AudioBufferDebugInfo& getDebugInfo() const { return mDebugInfo; }
    protected:
        void debugLoadFailure(const std::string& error);
        AudioBufferDebugInfo mDebugInfo;
        uint64_t mDebugId = 0;
#endif
    };
}
