#include "AudioSource.h"

namespace AV{
    AudioSource::AudioSource()
        : mAudioBuffer(0) {

    }

    AudioSource::~AudioSource(){

    }

    void AudioSource::play(){

    }

    void AudioSource::pause(){

    }

    void AudioSource::setAudioBuffer(AudioBufferPtr buffer){
        mAudioBuffer = buffer;
    }
}
