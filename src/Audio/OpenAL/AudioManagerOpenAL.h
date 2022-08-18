#pragma once

#include "Audio/AudioManager.h"

namespace AV{
    class AudioManagerOpenAL : public AudioManager{
    public:
        AudioManagerOpenAL();
        ~AudioManagerOpenAL();
    };
}
