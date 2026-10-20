#include "AudioManagerOpenAL.h"

#include "AudioSourceOpenAL.h"
#include "Audio/AudioBuffer.h"
#include "AudioBufferOpenAL.h"

#include "Logger/Log.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    AudioManagerOpenAL::AudioManagerOpenAL()
        : AudioManager(),
          mSetupSuccesful(false),
          mDevice(0),
          mDeviceName(""),
          mCtx(0) {

    }

    AudioManagerOpenAL::~AudioManagerOpenAL(){

    }

    void AudioManagerOpenAL::setup(){
#ifdef DEBUG_SERVER
        debugState().enabled = SystemSettings::isDebugServerEnabled();
        mSetupError.clear();
#endif
        //If audio is disabled, return early without setting up
        if(SystemSettings::getDisableAudio()) {
            return;
        }

        //Setup OpenAL here

        ALCdevice *device = alcOpenDevice(NULL);
#ifdef DEBUG_SERVER
        auto failed = [this](const char* message){
            mSetupError = message;
            debugState().record("setupFailed", 0, 0, "", message);
        };
#endif
        if(!device){
            AV_ERROR("Could not open an OpenAL device.");
#ifdef DEBUG_SERVER
            failed("Could not open an OpenAL device");
#endif
            return;
        }
        ALCcontext *ctx;

        ctx = alcCreateContext(device, NULL);
        if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
        {
            if(ctx != NULL){
                alcDestroyContext(ctx);
            }
            alcCloseDevice(device);
            AV_ERROR("Could not set an OpenAL context.");
#ifdef DEBUG_SERVER
            failed("Could not set an OpenAL context");
#endif
            return;
        }

        const ALCchar *name = NULL;
        if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
            name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
        if(!name || alcGetError(device) != AL_NO_ERROR)
            name = alcGetString(device, ALC_DEVICE_SPECIFIER);
        mDeviceName = name ? name : "unknown";
        AV_INFO("Opened \"{}\"", mDeviceName);

        setSetup(true);
        mSetupSuccesful = true;
        mCtx = ctx;
        mDevice = device;

        //Anything set before the context existed was only recorded, so apply it now.
        alListener3f(AL_POSITION, mListenerPosition.x, mListenerPosition.y, mListenerPosition.z);
        alListener3f(AL_VELOCITY, mListenerVelocity.x, mListenerVelocity.y, mListenerVelocity.z);
        alListenerfv(AL_ORIENTATION, mListenerOrientation);
        alListenerf(AL_GAIN, mVolume);
#ifdef DEBUG_SERVER
        debugState().record("setupSucceeded", 0, 0, "", mDeviceName);
#endif
    }

    void AudioManagerOpenAL::shutdown(){
        //Only shutdown if we were actually setup
        if(!isSetup()) {
            return;
        }

        alcMakeContextCurrent(NULL);
        if(mCtx) alcDestroyContext(mCtx);
        if(mDevice){
            alcCloseDevice(mDevice);
            AV_INFO("Closing audio device \"{}\"", mDeviceName);
        }

        setSetup(false);
        mCtx = nullptr;
        mDevice = nullptr;
    }

    AudioSourcePtr AudioManagerOpenAL::createAudioSource(const std::string& audioPath, AudioSourceType type){
        AudioSourcePtr audioSource = std::make_shared<AudioSourceOpenAL>(this);
        AudioBufferPtr buf = createAudioBuffer();
        if(type == AudioSourceType::Buffer){
            buf->load(audioPath);
        }else{
            assert(false && "Only buffer is implemented so far.");
        }
        audioSource->setAudioBuffer(buf);

        return audioSource;
    }

    AudioSourcePtr AudioManagerOpenAL::createAudioSourceFromBuffer(AudioBufferPtr bufPtr){
        AudioSourcePtr audioSource = std::make_shared<AudioSourceOpenAL>(this);
        audioSource->setAudioBuffer(bufPtr);

        return audioSource;
    }

    AudioBufferPtr AudioManagerOpenAL::createAudioBuffer(){
        AudioBufferPtr audioBuffer = std::make_shared<AudioBufferOpenAL>(this);

        return audioBuffer;
    }

    void AudioManagerOpenAL::setListenerPosition(Ogre::Vector3 pos){
        AudioManager::setListenerPosition(pos);
        if(!isSetup()) return;
        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
#ifdef DEBUG_SERVER
        debugOperation("setListenerPosition");
#endif
    }

    void AudioManagerOpenAL::setVolume(float volume){
        AudioManager::setVolume(volume);
        if(!isSetup()) return;
        alListenerf(AL_GAIN, volume);
#ifdef DEBUG_SERVER
        debugOperation("setVolume");
#endif
    }

    void AudioManagerOpenAL::setListenerVelocity(Ogre::Vector3 velocity){
        AudioManager::setListenerVelocity(velocity);
        if(!isSetup()) return;
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
#ifdef DEBUG_SERVER
        debugOperation("setListenerVelocity");
#endif
    }

    void AudioManagerOpenAL::setListenerOrientation(float vec[6]){
        AudioManager::setListenerOrientation(vec);
        if(!isSetup()) return;
        alListenerfv(AL_ORIENTATION, vec);
#ifdef DEBUG_SERVER
        debugOperation("setListenerOrientation");
#endif
    }

#ifdef DEBUG_SERVER
    AudioManagerSnapshot AudioManagerOpenAL::debugSnapshot() const{
        AudioManagerSnapshot out;
        out.backend = "OpenAL";
        out.device = mDeviceName;
        out.setupError = mSetupError;
        out.available = isSetup() && alcGetCurrentContext() == mCtx;
        if(!out.available) return out;
        //Read from OpenAL rather than the recorded values, as this reports what the backend holds.
        alGetListenerf(AL_GAIN, &out.gain);
        alGetListener3f(AL_POSITION, &out.position.x, &out.position.y, &out.position.z);
        alGetListener3f(AL_VELOCITY, &out.velocity.x, &out.velocity.y, &out.velocity.z);
        ALfloat orientation[6] = {};
        alGetListenerfv(AL_ORIENTATION, orientation);
        out.forward = Ogre::Vector3(orientation[0], orientation[1], orientation[2]);
        out.up = Ogre::Vector3(orientation[3], orientation[4], orientation[5]);
        out.distanceModel = alGetInteger(AL_DISTANCE_MODEL);
        return out;
    }

    void AudioManagerOpenAL::debugOperation(const char* operation){
        if(!debugState().enabled) return;
        const ALenum error = alGetError();
        if(error != AL_NO_ERROR){
            debugState().record("operationFailed", 0, 0, "",
                std::string(operation) + ": " + alGetString(error));
        }
    }
#endif

}
