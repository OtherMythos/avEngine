#include "AudioSourceOpenAL.h"

#include "AL/al.h"
#include "Audio/AudioBuffer.h"

namespace AV{
    AudioSourceOpenAL::AudioSourceOpenAL() : AudioSource() {
        alGenSources(1, &mSource);
    }

    AudioSourceOpenAL::~AudioSourceOpenAL(){

    }

    void AudioSourceOpenAL::setAudioBuffer(AudioBufferPtr buffer){
        alSourcei(mSource, AL_BUFFER, (ALint)buffer->getBuffer());
        assert(alGetError()==AL_NO_ERROR && "Failed to setup sound source");
    }

    void AudioSourceOpenAL::play(){
        alSourcePlay(mSource);
    }

    void AudioSourceOpenAL::pause(){
        alSourcePause(mSource);
    }
}
