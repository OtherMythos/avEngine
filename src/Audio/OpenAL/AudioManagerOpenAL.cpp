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
        //If audio is disabled, return early without setting up
        if(SystemSettings::getDisableAudio()) {
            return;
        }

        //Setup OpenAL here

        ALCdevice *device = alcOpenDevice(NULL);
        ALCcontext *ctx;

        ctx = alcCreateContext(device, NULL);
        if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
        {
            if(ctx != NULL){
                alcDestroyContext(ctx);
            }
            alcCloseDevice(device);
            AV_ERROR("Could not set an OpenAL context.");
            return;
        }

        const ALCchar *name = NULL;
        if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
            name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
        if(!name || alcGetError(device) != AL_NO_ERROR)
            name = alcGetString(device, ALC_DEVICE_SPECIFIER);
        mDeviceName = name;
        AV_INFO("Opened \"{}\"", name);

        setSetup(true);
        mSetupSuccesful = true;
        mCtx = ctx;
        mDevice = device;
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
        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    }

    Ogre::Vector3 AudioManagerOpenAL::getListenerPosition() const{
        ALfloat x, y, z;
        alGetListener3f(AL_POSITION, &x, &y, &z);

        return Ogre::Vector3(x, y, z);
    }

    float AudioManagerOpenAL::getVolume() const{
        ALfloat volume;
        alGetListenerf(AL_GAIN, &volume);

        return volume;
    }

    void AudioManagerOpenAL::setVolume(float volume){
        assert(volume >= 0.0 && volume <= 1.0);
        alListenerf(AL_GAIN, volume);
    }

    void AudioManagerOpenAL::setListenerVelocity(Ogre::Vector3 velocity){
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }

    Ogre::Vector3 AudioManagerOpenAL::getListenerVelocity() const{
        ALfloat x, y, z;
        alGetListener3f(AL_VELOCITY, &x, &y, &z);

        return Ogre::Vector3(x, y, z);
    }

    void AudioManagerOpenAL::setListenerOrientation(float vec[6]){
        alListenerfv(AL_ORIENTATION, vec);
    }

}
