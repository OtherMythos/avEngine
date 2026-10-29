#include "AudioManager.h"

#include "AudioSource.h"
#include "AudioBuffer.h"

#include <cassert>
#include <cstring>

namespace AV{
    AudioManager::AudioManager()
        : mNumAudioBuffers(0),
          mNumAudioSources(0),
          mSetup(false),
          mListenerPosition(Ogre::Vector3::ZERO),
          mListenerVelocity(Ogre::Vector3::ZERO),
          mListenerOrientation{0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f},
          mVolume(1.0f) {

    }

    void AudioManager::setListenerOrientation(float vec[6]){
        memcpy(mListenerOrientation, vec, sizeof(mListenerOrientation));
    }

    void AudioManager::setVolume(float volume){
        assert(volume >= 0.0 && volume <= 1.0);
        mVolume = volume;
    }

    AudioManager::~AudioManager(){
        for(auto* source : mSources) source->mManager = nullptr;
        for(auto* buffer : mBuffers) buffer->mManager = nullptr;
    }

    void AudioManager::shutdown(){
        for(auto* source : mSources) source->releaseBackend();
        for(auto* buffer : mBuffers) buffer->releaseBackend();
    }

    bool AudioManager::isBufferAttached(const AudioBuffer* buffer) const{
        for(auto* source : mSources) if(source->getAudioBuffer().get() == buffer) return true;
        return false;
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
