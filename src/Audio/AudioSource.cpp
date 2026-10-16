#include "AudioSource.h"

#include "AudioManager.h"
#include "AudioBuffer.h"

namespace AV{
    AudioSource::AudioSource(AudioManager* manager)
        : mManager(manager),
        mAudioBuffer(0) {
        mManager->mNumAudioSources++;
#ifdef DEBUG_SERVER
        mDebugId = mManager->debugState().addSource(this);
#endif
    }

    AudioSource::~AudioSource(){
#ifdef DEBUG_SERVER
        mManager->debugState().removeSource(mDebugId);
#endif
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
#ifdef DEBUG_SERVER
        debugCommand("bufferAssigned");
#endif
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

    void AudioSource::setRelative(bool relative){

    }
#ifdef DEBUG_SERVER
    void AudioSource::debugCommand(const char* command, const std::string& detail){
        mManager->debugState().record(command, mDebugId,
            mAudioBuffer ? mAudioBuffer->getDebugId() : 0,
            mAudioBuffer ? mAudioBuffer->getDebugInfo().path : "", detail);
    }
#endif
}
