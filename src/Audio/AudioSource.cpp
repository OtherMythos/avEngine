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

    void AudioSource::stop(){

    }

    void AudioSource::setPosition(const Ogre::Vector3& posVec){
        mPos = posVec;
    }

    void AudioSource::setVolume(float volume){

    }

    void AudioSource::setPitch(float pitch){

    }

    void AudioSource::setAudioBuffer(AudioBufferPtr buffer){
        mAudioBuffer = buffer;
    }

    void AudioSource::setLooping(bool looping){

    }

    void AudioSource::seekSeconds(float seconds){

    }

    void AudioSource::setRolloff(float rolloff){

    }

    void AudioSource::setDirection(const Ogre::Vector3& dirVec){

    }

    void AudioSource::setAttenuationDistance(float ref, float max){

    }

    void AudioSource::setVelocity(const Ogre::Vector3& vel){

    }
}
