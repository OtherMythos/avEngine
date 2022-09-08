#pragma once

#include "Audio/AudioBuffer.h"

#include "AL/al.h"

namespace AV{
    class AudioBufferOpenAL : public AudioBuffer{
    public:
        AudioBufferOpenAL(AudioManager* manager);
        ~AudioBufferOpenAL();

        virtual void load(const std::string& path);

    private:
        short *mMembuf;
        ALsizei mNumBytes;
        ALuint mBuffer = 0;

    public:
        unsigned int getBuffer() const { return mBuffer; }
    };
}
