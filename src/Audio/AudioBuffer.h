#pragma once

#include <string>

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
    };
}
