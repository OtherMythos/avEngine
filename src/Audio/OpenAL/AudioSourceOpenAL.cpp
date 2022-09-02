#include "AudioSourceOpenAL.h"

#include "AL/al.h"
#include "Audio/AudioBuffer.h"

namespace AV{
    AudioSourceOpenAL::AudioSourceOpenAL(AudioManager* manager) : AudioSource(manager) {
        alGenSources(1, &mSource);
    }

    AudioSourceOpenAL::~AudioSourceOpenAL(){

    }

    void AudioSourceOpenAL::setAudioBuffer(AudioBufferPtr buffer){
        alSourcei(mSource, AL_BUFFER, (ALint)buffer->getBuffer());
        assert(alGetError()==AL_NO_ERROR && "Failed to setup sound source");
        mAudioBuffer = buffer;
    }

    void AudioSourceOpenAL::play(){
        alSourcePlay(mSource);
    }

    void AudioSourceOpenAL::pause(){
        alSourcePause(mSource);
    }

    void AudioSourceOpenAL::stop(){
        alSourceStop(mSource);
    }

    void AudioSourceOpenAL::setVolume(float volume){
        alSourcef(mSource, AL_GAIN, volume);
    }

    void AudioSourceOpenAL::setPosition(const Ogre::Vector3& posVec){
        alSource3f(mSource, AL_POSITION, posVec.x, posVec.y, posVec.z);
    }

    void AudioSourceOpenAL::setLooping(bool looping){
        alSourcei(mSource, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    }

    void AudioSourceOpenAL::setPitch(float pitch){
        alSourcef(mSource, AL_PITCH, pitch);
    }

    void AudioSourceOpenAL::seekSeconds(float seconds){
        alSourcef(mSource, AL_SEC_OFFSET, seconds);
    }

    void AudioSourceOpenAL::setRolloff(float rolloff){
        alSourcef(mSource, AL_ROLLOFF_FACTOR, rolloff);
    }

    void AudioSourceOpenAL::setDirection(const Ogre::Vector3& dirVec){
        alSource3f(mSource, AL_DIRECTION, dirVec.x, dirVec.y, dirVec.z);
    }

    void AudioSourceOpenAL::setAttenuationDistance(float ref, float max){
        alSourcef(mSource, AL_REFERENCE_DISTANCE, ref);
        alSourcef(mSource, AL_MAX_DISTANCE, max);
    }

    void AudioSourceOpenAL::setVelocity(const Ogre::Vector3& vel){
        alSource3f(mSource, AL_VELOCITY, vel.x, vel.y, vel.z);
    }
}
