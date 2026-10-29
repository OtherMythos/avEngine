#include "AudioSource.h"

#include "AudioManager.h"
#include "AudioBuffer.h"

namespace AV{
    AudioSource::AudioSource(AudioManager* manager)
        : mManager(manager),
        mAudioBuffer(0) {
        mManager->mNumAudioSources++;
        mManager->mSources.insert(this);
#ifdef DEBUG_SERVER
        mDebugId = mManager->debugState().addSource(this);
#endif
    }

    AudioSource::~AudioSource(){
        if(!mManager) return;
        mManager->mSources.erase(this);
#ifdef DEBUG_SERVER
        mManager->debugState().removeSource(mDebugId);
#endif
        mManager->mNumAudioSources--;
    }

    double AudioSource::getDurationSeconds() const{
        return mAudioBuffer ? mAudioBuffer->getDurationSeconds() : -1;
    }

    std::string AudioSource::getError() const{
        return mAudioBuffer ? mAudioBuffer->getError() : "";
    }

    std::string AudioSource::getAudioPath() const{
#ifdef DEBUG_SERVER
        if(mAudioBuffer && mAudioBuffer->getPath().empty()) return mAudioBuffer->getDebugInfo().path;
#endif
        return mAudioBuffer ? mAudioBuffer->getPath() : mSourcePath;
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
        if(!mManager) return;
        mManager->debugState().record(command, mDebugId,
            mAudioBuffer ? mAudioBuffer->getDebugId() : 0,
            mAudioBuffer ? mAudioBuffer->getDebugInfo().path : "", detail);
    }
#endif
}
