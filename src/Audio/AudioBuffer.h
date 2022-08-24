#pragma once

#include <string>

namespace AV{
    class AudioBuffer{
    public:
        AudioBuffer();
        ~AudioBuffer();

        virtual void play();
        virtual void pause();

        virtual void load(const std::string& path);

    protected:
        bool mBufferReady;

    public:
        virtual unsigned int getBuffer() const { return 0; }
    };
}
