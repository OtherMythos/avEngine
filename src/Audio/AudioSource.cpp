#include "AudioSource.h"

#include "AudioManager.h"

namespace AV{
    AudioSource::AudioSource(AudioManager* manager)
        : mManager(manager),
        mAudioBuffer(0) {
        mManager->mNumAudioSources++;
    }

    AudioSource::~AudioSource(){
        mManager->mNumAudioSources--;
    }

    void AudioSource::play(){

    }

    void AudioSource::pause(){

    }

    void AudioSource::setAudioBuffer(AudioBufferPtr buffer){
        mAudioBuffer = buffer;
    }
}
