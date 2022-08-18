#pragma once

#include "Audio/AudioSource.h"

namespace AV{
    class AudioSourceOpenAL : public AudioSource{
    public:
        AudioSourceOpenAL();
        ~AudioSourceOpenAL();
    };
}