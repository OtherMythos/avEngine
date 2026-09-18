#include "AudioSourceOpenAL.h"

#include "AL/al.h"
#include "Audio/AudioBuffer.h"
#include "Audio/AudioManager.h"

namespace AV{
    AudioSourceOpenAL::AudioSourceOpenAL(AudioManager* manager) : AudioSource(manager) {
        if(!mManager->isSetup()) return;
        alGenSources(1, &mSource);
#ifdef DEBUG_SERVER
        debugOperation("createSource");
#endif
    }

    AudioSourceOpenAL::~AudioSourceOpenAL(){

    }

    void AudioSourceOpenAL::setAudioBuffer(AudioBufferPtr buffer){
        if(mSource && mManager->isSetup()){
            alSourcei(mSource, AL_BUFFER, buffer ? (ALint)buffer->getBuffer() : 0);
            const ALenum error = alGetError();
            if(error != AL_NO_ERROR){
#ifdef DEBUG_SERVER
                debugCommand("operationFailed", std::string("setAudioBuffer: ") + alGetString(error));
#endif
                return;
            }
        }
        AudioSource::setAudioBuffer(buffer);
#ifdef DEBUG_SERVER
        mManager->debugState().observe(this);
#endif
    }

    void AudioSourceOpenAL::play(){
#ifdef DEBUG_SERVER
        debugCommand("play");
#endif
        if(!mSource || !mManager->isSetup()) return;
        alSourcePlay(mSource);
#ifdef DEBUG_SERVER
        debugOperation("play", true);
#endif
    }

    void AudioSourceOpenAL::pause(){
#ifdef DEBUG_SERVER
        debugCommand("pause");
#endif
        if(!mSource || !mManager->isSetup()) return;
        alSourcePause(mSource);
#ifdef DEBUG_SERVER
        debugOperation("pause", true);
#endif
    }

    void AudioSourceOpenAL::stop(){
#ifdef DEBUG_SERVER
        debugCommand("stop");
#endif
        if(!mSource || !mManager->isSetup()) return;
        alSourceStop(mSource);
#ifdef DEBUG_SERVER
        debugOperation("stop", true);
#endif
    }

    void AudioSourceOpenAL::setVolume(float volume){
        if(!mSource || !mManager->isSetup()) return;
        alSourcef(mSource, AL_GAIN, volume);
#ifdef DEBUG_SERVER
        debugOperation("setVolume");
#endif
    }

    void AudioSourceOpenAL::setPosition(const Ogre::Vector3& posVec){
        AudioSource::setPosition(posVec);
        if(!mSource || !mManager->isSetup()) return;
        alSource3f(mSource, AL_POSITION, posVec.x, posVec.y, posVec.z);
#ifdef DEBUG_SERVER
        debugOperation("setPosition");
#endif
    }

    void AudioSourceOpenAL::setLooping(bool looping){
        if(!mSource || !mManager->isSetup()) return;
        alSourcei(mSource, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
#ifdef DEBUG_SERVER
        debugOperation("setLooping");
#endif
    }

    void AudioSourceOpenAL::setPitch(float pitch){
        if(!mSource || !mManager->isSetup()) return;
        alSourcef(mSource, AL_PITCH, pitch);
#ifdef DEBUG_SERVER
        debugOperation("setPitch");
#endif
    }

    void AudioSourceOpenAL::seekSeconds(float seconds){
#ifdef DEBUG_SERVER
        debugCommand("seek", std::to_string(seconds));
#endif
        if(!mSource || !mManager->isSetup()) return;
        alSourcef(mSource, AL_SEC_OFFSET, seconds);
#ifdef DEBUG_SERVER
        debugOperation("seekSeconds", true);
#endif
    }

    void AudioSourceOpenAL::setRolloff(float rolloff){
        if(!mSource || !mManager->isSetup()) return;
        alSourcef(mSource, AL_ROLLOFF_FACTOR, rolloff);
#ifdef DEBUG_SERVER
        debugOperation("setRolloff");
#endif
    }

    void AudioSourceOpenAL::setDirection(const Ogre::Vector3& dirVec){
        if(!mSource || !mManager->isSetup()) return;
        alSource3f(mSource, AL_DIRECTION, dirVec.x, dirVec.y, dirVec.z);
#ifdef DEBUG_SERVER
        debugOperation("setDirection");
#endif
    }

    void AudioSourceOpenAL::setAttenuationDistance(float ref, float max){
        if(!mSource || !mManager->isSetup()) return;
        alSourcef(mSource, AL_REFERENCE_DISTANCE, ref);
        alSourcef(mSource, AL_MAX_DISTANCE, max);
#ifdef DEBUG_SERVER
        debugOperation("setAttenuationDistance");
#endif
    }

    void AudioSourceOpenAL::setVelocity(const Ogre::Vector3& vel){
        if(!mSource || !mManager->isSetup()) return;
        alSource3f(mSource, AL_VELOCITY, vel.x, vel.y, vel.z);
#ifdef DEBUG_SERVER
        debugOperation("setVelocity");
#endif
    }

    void AudioSourceOpenAL::setRelative(bool relative){
        if(!mSource || !mManager->isSetup()) return;
        alSourcei(mSource, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
#ifdef DEBUG_SERVER
        debugOperation("setRelative");
#endif
    }

#ifdef DEBUG_SERVER
    void AudioSourceOpenAL::debugOperation(const char* operation, bool observe){
        if(!mManager->debugState().enabled) return;
        const ALenum error = alGetError();
        if(error != AL_NO_ERROR){
            debugCommand("operationFailed", std::string(operation) + ": " + alGetString(error));
        }
        if(observe) mManager->debugState().observe(this);
    }

    std::string AudioSourceOpenAL::debugPlaybackState() const{
        if(!mManager->isSetup() || !mSource || !alIsSource(mSource)) return "unavailable";
        ALint state = 0;
        alGetSourcei(mSource, AL_SOURCE_STATE, &state);
        switch(state){
            case AL_INITIAL: return "initial";
            case AL_PLAYING: return "playing";
            case AL_PAUSED: return "paused";
            case AL_STOPPED: return "stopped";
            default: return "unavailable";
        }
    }

    AudioSourceSnapshot AudioSourceOpenAL::debugSnapshot() const{
        AudioSourceSnapshot out;
        out.state = debugPlaybackState();
        if(out.state == "unavailable") return out;
        ALint looping = 0, relative = 0;
        out.reason.clear();
        alGetSourcei(mSource, AL_LOOPING, &looping);
        alGetSourcei(mSource, AL_SOURCE_RELATIVE, &relative);
        out.looping = looping == AL_TRUE;
        out.relative = relative == AL_TRUE;
        alGetSourcef(mSource, AL_SEC_OFFSET, &out.offset);
        alGetSourcef(mSource, AL_GAIN, &out.gain);
        alGetSourcef(mSource, AL_PITCH, &out.pitch);
        alGetSourcef(mSource, AL_ROLLOFF_FACTOR, &out.rolloff);
        alGetSourcef(mSource, AL_REFERENCE_DISTANCE, &out.referenceDistance);
        alGetSourcef(mSource, AL_MAX_DISTANCE, &out.maxDistance);
        alGetSourcef(mSource, AL_MIN_GAIN, &out.minGain);
        alGetSourcef(mSource, AL_MAX_GAIN, &out.maxGain);
        alGetSourcef(mSource, AL_CONE_INNER_ANGLE, &out.coneInnerAngle);
        alGetSourcef(mSource, AL_CONE_OUTER_ANGLE, &out.coneOuterAngle);
        alGetSourcef(mSource, AL_CONE_OUTER_GAIN, &out.coneOuterGain);
        alGetSource3f(mSource, AL_POSITION, &out.position.x, &out.position.y, &out.position.z);
        alGetSource3f(mSource, AL_VELOCITY, &out.velocity.x, &out.velocity.y, &out.velocity.z);
        alGetSource3f(mSource, AL_DIRECTION, &out.direction.x, &out.direction.y, &out.direction.z);
        return out;
    }
#endif
}
