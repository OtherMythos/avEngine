#pragma once

#include <string>
#include "AL/al.h"

//TODO shift this off to an OpenAL implementation.

namespace AV{
    class AudioBuffer{
    public:
        AudioBuffer();
        ~AudioBuffer();

        virtual void play();
        virtual void pause();

        virtual void load(const std::string& path);

    private:
        short *mMembuf;
        ALsizei mNumBytes;
        bool mBufferReady;
        ALuint mBuffer = 0;

    public:
        const ALuint getBuffer() const { return mBuffer; }
    };
}
