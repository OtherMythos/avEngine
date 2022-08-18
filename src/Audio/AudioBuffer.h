#pragma once

namespace AV{
    class AudioBuffer{
    public:
        AudioBuffer();
        ~AudioBuffer();

        virtual void play();
        virtual void pause();
    };
}
