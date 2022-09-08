#include "AudioManager.h"

#include "AudioSource.h"
#include "AudioBuffer.h"

namespace AV{
    AudioManager::AudioManager()
        : mNumAudioBuffers(0),
          mNumAudioSources(0) {

    }

    AudioManager::~AudioManager(){

    }

    AudioBufferPtr AudioManager::createAudioBuffer(){
        AudioBufferPtr audioBuffer = std::make_shared<AudioBuffer>(this);

        return audioBuffer;
    }

    AudioSourcePtr AudioManager::createAudioSource(const std::string& audioPath, AudioSourceType type){
        AudioSourcePtr audioSource = std::make_shared<AudioSource>(this);
        AudioBufferPtr buf = createAudioBuffer();
        audioSource->setAudioBuffer(buf);

        return audioSource;
    }

    AudioSourcePtr AudioManager::createAudioSourceFromBuffer(AudioBufferPtr bufPtr){
        AudioSourcePtr audioSource = std::make_shared<AudioSource>(this);
        audioSource->setAudioBuffer(bufPtr);

        return audioSource;
    }
}
