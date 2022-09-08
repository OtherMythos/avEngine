#include "AudioBuffer.h"

#include "AudioManager.h"

namespace AV{
    AudioBuffer::AudioBuffer(AudioManager* manager)
        : mManager(manager),
          mBufferReady(false) {

        mManager->mNumAudioBuffers++;
    }

    AudioBuffer::~AudioBuffer(){
        mManager->mNumAudioBuffers--;
    }

    void AudioBuffer::play(){

    }

    void AudioBuffer::pause(){

    }

    void AudioBuffer::load(const std::string& path){

    }
}
