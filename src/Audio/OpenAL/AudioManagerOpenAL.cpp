#include "AudioManagerOpenAL.h"

#include "AudioSourceOpenAL.h"
#include "Audio/AudioBuffer.h"

#include "Logger/Log.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

namespace AV{
    AudioManagerOpenAL::AudioManagerOpenAL()
        : AudioManager(),
          mSetupSuccesful(false),
          mDevice(0),
          mCtx(0) {

    }

    AudioManagerOpenAL::~AudioManagerOpenAL(){

    }

    void AudioManagerOpenAL::setup(){
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
        AV_INFO("Opened \"{}\"\n", name);

        mSetupSuccesful = true;
    }

    void AudioManagerOpenAL::shutdown(){
        assert(mCtx);
        assert(mDevice);

        alcMakeContextCurrent(NULL);
        alcDestroyContext(mCtx);
        alcCloseDevice(mDevice);
    }

    AudioSourcePtr AudioManagerOpenAL::createAudioSource(const std::string& audioPath, AudioSourceType type){
        AudioSourcePtr audioSource = std::make_shared<AudioSourceOpenAL>();
        AudioBufferPtr buf = createAudioBuffer();
        if(type == AudioSourceType::Buffer){
            buf->load(audioPath);
        }else{
            assert(false && "Only buffer is implemented so far.");
        }
        audioSource->setAudioBuffer(buf);

        return audioSource;
    }
}
