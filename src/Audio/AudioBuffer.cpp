#include "AudioBuffer.h"

#include "AudioManager.h"

namespace AV{
    AudioBuffer::AudioBuffer(AudioManager* manager)
        : mManager(manager),
          mBufferReady(false) {

        mManager->mNumAudioBuffers++;
#ifdef DEBUG_SERVER
        mDebugId = mManager->debugState().addBuffer(this);
#endif
    }

    AudioBuffer::~AudioBuffer(){
#ifdef DEBUG_SERVER
        mManager->debugState().removeBuffer(mDebugId);
#endif
        mManager->mNumAudioBuffers--;
    }

    void AudioBuffer::play(){

    }

    void AudioBuffer::pause(){

    }

    void AudioBuffer::load(const std::string& path){

    }
#ifdef DEBUG_SERVER
    void AudioBuffer::debugLoadFailure(const std::string& error){
        if(!mManager->debugState().enabled) return;
        mDebugInfo.loadError = error;
        mManager->debugState().record("loadFailed", 0, mDebugId, mDebugInfo.lastAttemptPath, error);
    }
#endif
}
