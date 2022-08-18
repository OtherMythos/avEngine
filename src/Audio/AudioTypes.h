#pragma once

#include <memory>

namespace AV{
    class AudioSource;
    class AudioBuffer;

    typedef std::shared_ptr<AudioSource> AudioSourcePtr;
    typedef std::shared_ptr<AudioBuffer> AudioBufferPtr;

    enum class AudioSourceType{
        Buffer,
        Stream
    };
}
