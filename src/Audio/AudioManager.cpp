#include "AudioManager.h"

#include "AudioSource.h"
#include "AudioBuffer.h"

namespace AV{
    AudioManager::AudioManager(){

    }

    AudioManager::~AudioManager(){

    }

    void AudioManager::play(){

    }

    void AudioManager::pause(){

    }

    AudioBufferPtr AudioManager::createAudioBuffer(){
        AudioBufferPtr audioBuffer = std::make_shared<AudioBuffer>();

        return audioBuffer;
    }

    AudioSourcePtr AudioManager::createAudioSource(const std::string& audioPath, AudioSourceType type){
        AudioSourcePtr audioSource = std::make_shared<AudioSource>();
        AudioBufferPtr buf = createAudioBuffer();
        audioSource->setAudioBuffer(buf);

        return audioSource;
    }
}
